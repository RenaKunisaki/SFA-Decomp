/*
 * Unused GroundBaddie-derived enemy in the ChukChuk/IceBall family.
 *
 * Slot 202 has no retail object name. The iceBaddie namespace is descriptive;
 * see docs/dll_00CA_rename_proposal.md for the supporting retail-data audit.
 */
#include "dlls/objects/202.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/math_api.h"
#include "game/objects/object.h"
#include "game/objects/object_setup.h"
#include "main/audio/sfx_play_api.h"
#include "main/audio/sfx_trigger_ids.h"
#include "main/camera.h"
#include "main/camera_shake_api.h"
#include "main/dll/baddie_control_interface.h"
#include "main/dll/partfx_interface.h"
#include "main/frame_timing.h"
#include "main/gamebits_api.h"
#include "main/mapEventTypes.h"
#include "main/object_render.h"
#include "main/obj_group.h"
#include "main/obj_message.h"
#include "main/obj_path.h"
#include "main/objanim.h"
#include "main/objhits.h"
#include "main/objprint_api.h"
#include "main/objseq.h"
#include "main/player_control_interface.h"
#include "main/vecmath.h"
#include "main/voxmaps.h"
#include "string.h"
#include "sys/objects.h"
#include "sys/objects/lifecycle.h"
#include "main/dll/baddie_state.h"
#include "main/dll/dll_00C9_enemy.h"
#include "main/dll/wispbaddie_baddie.h"
#include "main/audio/sfx_position_api.h"
#include "main/audio/sfx_ids.h"
#include "main/dll/baddie_setmove.h"
#include "main/pad_api.h"
#include "main/dll/seqobj11d_ext.h"
#include "main/dll/wispbaddieseq_ext.h"
#include "main/gameloop_api.h"
#include "main/audio/sfx.h"
#include "main/dll/curve_walker.h"
#include "main/dll/rom_curve_interface.h"
#include "main/gamebits.h"
#include "main/dll/objfsa.h"
#include "main/gamebit_ids.h"
#include "main/dll/newseqobj_baddie.h"
#include "main/dll/baddie_frozen.h"
#include "main/game_ui_interface.h"
#include "main/dll/tricky_api.h"
#include "main/model.h"
#include "main/object_transform.h"
#include "main/dll/player_target.h"
#include "main/dll/player_api.h"
#include "dlls/objects/225_WispBaddie.h"
#include "dolphin/MSL_C/PPCEABI/bare/H/trig_float_helpers.h"
#include "main/obj_link.h"
#include "main/objfx.h"
#include "main/objtexture.h"
#include "main/dll/seqObj11E.h"
#include "main/dll/groundbaddiepush_ext.h"
#include "main/dll/dll_00C9_enemy_ext.h"
#include "dlls/objects/336_GCRobotLigh.h"
#include "dolphin/mtx.h"
#include "main/dll/mikaladon.h"
#include "main/dll/magicPlant.h"
#include "main/dll/kooshy.h"
#include "main/dll/weevil.h"
#include "main/trig.h"
#include "main/dll/waterfx_interface.h"
#include "main/dll/fall_ladders.h"
#include "main/dll/fireflyLantern.h"
#include "main/dll/duster_api.h"
#include "main/track_bbox_api.h"
#include "main/sky_interface.h"
#include "main/dll/duster.h"
#include "dlls/objects/216_PinPonSpike.h"
#include "main/dll/duster_wb.h"
#include "main/obj_query.h"
#include "main/dll/hoodedzyck.h"
#include "main/camera_interface.h"
#include "main/model_light.h"
#include "main/dll/firecrawler.h"
#include "main/dll/dll_0273_firepipe.h"
#include "main/dll/hagabon_mk2.h"
#include "main/dll/snowworm.h"
#include "main/dll/baddiewhirlpool.h"
#include "track/intersect_whirlpool_api.h"

/* Baddie-family animation data shared with the sequence-driver TUs. */
typedef struct FamilyTable
{
    u8* tbl0;
    u8* tbl4;
    u8* tbl8;
    u8* tblC;
    u8* tbl10;
    u8* tbl14;
    u8* tbl18;
    u8* tbl1c;
    u8* tbl20;
    u8* tbl24;
} FamilyTable;

u8 gBaddieMoveProgressTable[288] = {
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 61, 163, 215, 10, 0, 0, 0, 0, 61, 163,
    215, 10,  61, 204, 204, 205, 61, 204, 204, 205, 0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  61,
    163, 215, 10, 61,  163, 215, 10, 61,  35,  215, 10, 61, 35, 215, 10, 0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0,  0,   0,   0,  0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,   0,   0,   0,  0,   0,   0,   0,  0,  0,  0,   0,  0, 0, 0};
u8 gSharpClawLocomotionMoves[48] = {60, 35, 215, 10, 0, 0, 0, 0, 0,  0, 0, 0, 60, 35, 215, 10, 0, 0, 0, 0, 11, 0, 0, 0,
                       60, 35, 215, 10, 0, 0, 0, 0, 15, 0, 0, 0, 60, 35, 215, 10, 0, 0, 0, 0, 12, 0, 0, 0};
u8 gSharpClawIdleMoveTable[324] = {
    0,  0,  0,  0,  0, 0, 0,  0,  13, 0,  0, 0, 64, 64,  0,  0,  0, 0, 0,  0,  0,  0,  0, 0, 64, 64,  0,  0,  0, 0,
    0,  0,  0,  0,  0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   0,  0,  0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   0,  0,  0, 0,
    64, 64, 0,  0,  0, 0, 0,  0,  0,  0,  0, 0, 64, 64,  0,  0,  0, 0, 0,  0,  0,  0,  0, 0, 64, 64,  0,  0,  0, 0,
    0,  0,  0,  0,  0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   0,  0,  0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   3,  0,  0, 0,
    64, 64, 0,  0,  0, 0, 0,  0,  3,  0,  0, 0, 64, 64,  0,  0,  0, 0, 0,  0,  6,  0,  0, 0, 64, 64,  0,  0,  0, 0,
    0,  0,  4,  0,  0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   5,  0,  0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   22, 25, 0, 0,
    64, 64, 0,  0,  0, 0, 0,  0,  6,  25, 0, 0, 63, 192, 0,  0,  0, 0, 0,  0,  24, 25, 0, 0, 63, 192, 0,  0,  0, 0,
    0,  0,  45, 25, 0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   27, 26, 0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   3,  25, 0, 0,
    64, 64, 0,  0,  0, 0, 0,  0,  7,  25, 0, 0, 64, 160, 0,  0,  0, 0, 0,  0,  26, 25, 0, 0, 64, 64,  0,  0,  0, 0,
    0,  0,  8,  25, 0, 0, 64, 0,  0,  0,  0, 0, 0,  0,   23, 25, 0, 0, 64, 64, 0,  0,  0, 0, 0,  0,   3,  25, 0, 0,
    64, 0,  0,  0,  0, 0, 0,  1,  11, 0,  0, 0, 64, 128, 0,  0,  0, 0, 0,  0,  30, 25, 0, 0};
u8 gSharpClawAnimEventMoves[300] = {
    63, 128, 0,  0, 0, 0, 0,  11,  64, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  60, 0, 0, 0, 63, 128, 0,  0, 0, 0,
    0,  11,  61, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  61, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  62, 0, 0, 0,
    63, 128, 0,  0, 0, 0, 0,  11,  62, 0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0,
    0,  0,   0,  0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  64, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  63, 0, 0, 0,
    63, 128, 0,  0, 0, 0, 0,  11,  61, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  61, 0, 0, 0, 63, 128, 0,  0, 0, 0,
    0,  11,  62, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  62, 0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0,
    0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  64, 0, 0, 0, 63, 128, 0,  0, 0, 0,
    0,  11,  60, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  61, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  61, 0, 0, 0,
    63, 128, 0,  0, 0, 0, 0,  11,  62, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  62, 0, 0, 0, 0,  0,   0,  0, 0, 0,
    0,  11,  0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  64, 0, 0, 0};
u8 gSharpClawBlockReactionMoves[300] = {62, 148, 122, 225, 0, 0, 0, 11, 69, 2, 2, 0, 62, 148, 122, 225, 0, 0, 0, 11, 65, 2, 2, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 66, 2, 2, 0, 62, 148, 122, 225, 0, 0, 0, 11, 66, 2, 2, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 67, 2, 2, 0, 62, 148, 122, 225, 0, 0, 0, 11, 67, 2, 2, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 69, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 68, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 66, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 66, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 67, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 67, 2, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 69, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 65, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 66, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 66, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 67, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 67, 2, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 69, 2, 0, 0};
u8 gSharpClawAttackHitVolumes[36] = {0, 0,  0,  0, 0, 0, 0, 11, 24, 1, 0, 0, 0, 0,  0,  0, 0, 0,
                       0, 12, 25, 1, 0, 0, 0, 0,  0,  0, 0, 0, 0, 10, 16, 1, 0, 0};
u8 gSharpClawModeIdleMoves[96] = {63, 128, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 63, 128, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0,
                       63, 0,   0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0,  0,   0, 0, 0, 0, 0, 0, 0,  0, 0, 0,
                       0,  0,   0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 63, 0,   0, 0, 0, 0, 0, 0, 21, 0, 0, 0,
                       63, 128, 0, 0, 0, 0, 0, 0, 21, 0, 0, 0, 63, 128, 0, 0, 0, 0, 0, 0, 21, 0, 0, 0};
u8 gSharpClawHitReactionMoves[300] = {63, 0,   0,   0,   0, 0, 0, 0,  40, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 38, 0, 0, 0,
                        63, 76,  204, 205, 0, 0, 0, 1,  53, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 47, 0, 0, 0,
                        63, 76,  204, 205, 0, 0, 0, 1,  54, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 48, 0, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0,
                        63, 192, 0,   0,   0, 0, 0, 0,  57, 7, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 38, 9, 0, 0,
                        64, 0,   0,   0,   0, 0, 0, 1,  32, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 49, 9, 0, 0,
                        63, 0,   0,   0,   0, 0, 0, 0,  57, 7, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 50, 9, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0,
                        63, 76,  204, 205, 0, 0, 0, 0,  39, 3, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 57, 7, 0, 0,
                        63, 153, 153, 154, 0, 0, 0, 0,  42, 1, 0, 0, 63, 153, 153, 154, 0, 0, 0, 0, 42, 1, 0, 0,
                        63, 153, 153, 154, 0, 0, 0, 0,  41, 2, 0, 0, 63, 153, 153, 154, 0, 0, 0, 0, 41, 2, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0,
                        63, 192, 0,   0,   0, 0, 0, 11, 28, 3, 0, 0};
u8 gSharpClawReactionMoveChain[208] = {
    0,  0,   0,  0,  0, 0, 0, 0,  0,  0, 0, 0,  0, 0, 0, 0,  63, 192, 0,  0,  0, 0, 0, 11, 56, 1, 5, 10, 0, 0, 0, 64,
    63, 192, 0,  0,  0, 0, 0, 11, 55, 2, 6, 11, 0, 0, 0, 64, 63, 192, 0,  0,  0, 0, 0, 11, 29, 0, 0, 0,  0, 0, 0, 0,
    63, 192, 0,  0,  0, 0, 0, 3,  46, 0, 0, 0,  0, 0, 0, 0,  63, 192, 0,  0,  0, 0, 0, 11, 51, 0, 0, 0,  0, 0, 0, 0,
    63, 192, 0,  0,  0, 0, 0, 11, 52, 0, 0, 0,  0, 0, 0, 0,  63, 192, 0,  0,  0, 0, 0, 11, 59, 7, 8, 12, 0, 0, 0, 64,
    63, 64,  0,  0,  0, 0, 0, 11, 58, 0, 0, 0,  0, 0, 0, 0,  63, 128, 0,  0,  0, 0, 0, 11, 36, 0, 0, 0,  0, 0, 0, 0,
    63, 51,  51, 51, 0, 0, 0, 11, 70, 0, 0, 0,  0, 0, 0, 0,  63, 51,  51, 51, 0, 0, 0, 11, 70, 0, 0, 0,  0, 0, 0, 0,
    63, 51,  51, 51, 0, 0, 0, 11, 71, 0, 0, 0,  0, 0, 0, 0};
u8 gSharpClawMoveSelectTable[432] = {
    0,  0,   0,   0,   0, 0, 0, 0, 21, 0, 0, 0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 36, 0, 0,  0,   0, 0, 0, 0,
    63, 128, 0,   0,   0, 0, 0, 0, 24, 0, 5, 230, 1, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0, 25, 0, 9,  230, 1, 0, 0, 0,
    63, 128, 0,   0,   0, 0, 0, 0, 36, 0, 0, 0,   0, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0, 25, 0, 9,  230, 1, 0, 0, 0,
    64, 0,   0,   0,   0, 0, 0, 0, 7,  0, 0, 0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 36, 0, 0,  0,   0, 0, 0, 0,
    63, 128, 0,   0,   0, 0, 0, 0, 24, 0, 5, 230, 1, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0, 5,  230, 1, 0, 0, 0,
    63, 38,  102, 102, 0, 4, 0, 0, 17, 0, 0, 0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 2, 0, 0, 18, 0, 0,  0,   0, 0, 0, 0,
    63, 38,  102, 102, 0, 2, 0, 0, 18, 0, 0, 0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 2,  0, 0,  0,   0, 0, 0, 0,
    63, 166, 102, 102, 0, 0, 0, 0, 25, 0, 9, 230, 1, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0, 5,  230, 1, 0, 0, 0,
    63, 38,  102, 102, 0, 8, 0, 0, 19, 0, 0, 0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 1, 0, 0, 16, 0, 33, 230, 2, 0, 0, 0,
    63, 38,  102, 102, 0, 8, 0, 0, 19, 0, 0, 0,   0, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0, 25, 0, 9,  230, 1, 0, 0, 0,
    63, 128, 0,   0,   0, 0, 0, 0, 24, 0, 5, 230, 1, 0, 0, 0, 63, 38,  102, 102, 0, 2, 0, 0, 18, 0, 0,  0,   0, 0, 0, 0,
    63, 38,  102, 102, 0, 4, 0, 0, 17, 0, 0, 0,   0, 0, 0, 0, 63, 140, 204, 205, 0, 1, 0, 0, 16, 0, 33, 230, 2, 0, 0, 0,
    63, 12,  204, 205, 0, 1, 0, 0, 12, 0, 0, 0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 8, 0, 0, 19, 0, 0,  0,   0, 0, 0, 0,
    63, 38,  102, 102, 0, 2, 0, 0, 18, 0, 0, 0,   0, 0, 0, 0};
u8 gSharpClawDeflectHitboxFlags[24] = {0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
u8 gSharpClawSoMoveSelectTable[432] = {0,  0,   0,   0,   0, 0, 0, 0, 21, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0,
                        36, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0,
                        63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0, 63, 128, 0,   0,   0, 1, 0, 0,
                        16, 0,   33,  230, 2, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0,
                        63, 128, 0,   0,   0, 1, 0, 0, 16, 0,   33,  230, 2, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0,
                        36, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0,
                        63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0, 63, 38,  102, 102, 0, 4, 0, 0,
                        17, 0,   0,   0,   0, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0,
                        63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0,
                        2,  0,   0,   0,   0, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0,
                        63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0, 63, 38,  102, 102, 0, 8, 0, 0,
                        19, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 1, 0, 0, 16, 0,   33,  230, 2, 0, 0, 0,
                        63, 38,  102, 102, 0, 8, 0, 0, 19, 0,   0,   0,   0, 0, 0, 0, 63, 166, 102, 102, 0, 0, 0, 0,
                        25, 0,   9,   230, 1, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0,
                        63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 4, 0, 0,
                        17, 0,   0,   0,   0, 0, 0, 0, 63, 140, 204, 205, 0, 1, 0, 0, 16, 0,   33,  230, 2, 0, 0, 0,
                        63, 12,  204, 205, 0, 1, 0, 0, 12, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 8, 0, 0,
                        19, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0, 0, 0, 0};
u8 gSharpClawSoDeflectHitboxFlags[24] = {0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0};
u8 gSharpClawCoAttackHitVolumes[36] = {0, 0,  0,  0, 0, 0, 0, 11, 24, 2, 0, 0, 0, 0,  0,  0, 0, 0,
                       0, 10, 25, 2, 0, 0, 0, 0,  0,  0, 0, 0, 0, 24, 16, 4, 0, 0};
u8 gSharpClawCoMoveSelectTable[432] = {0,  0,   0,   0,   0, 0, 0, 0, 21, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0,
                        36, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0,
                        63, 102, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0, 63, 128, 0,   0,   0, 1, 0, 0,
                        16, 0,   33,  230, 2, 0, 0, 0, 63, 102, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0,
                        63, 128, 0,   0,   0, 1, 0, 0, 16, 0,   33,  230, 2, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0,
                        36, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0,
                        63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0, 63, 38,  102, 102, 0, 4, 0, 0,
                        17, 0,   0,   0,   0, 0, 0, 0, 63, 102, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0,
                        63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0,
                        2,  0,   0,   0,   0, 0, 0, 0, 63, 102, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1, 0, 0, 0,
                        63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0, 63, 38,  102, 102, 0, 8, 0, 0,
                        19, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0, 1, 0, 0, 16, 0,   33,  230, 2, 0, 0, 0,
                        63, 38,  102, 102, 0, 8, 0, 0, 19, 0,   0,   0,   0, 0, 0, 0, 63, 102, 102, 102, 0, 0, 0, 0,
                        25, 0,   9,   230, 1, 0, 0, 0, 63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1, 0, 0, 0,
                        63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 4, 0, 0,
                        17, 0,   0,   0,   0, 0, 0, 0, 63, 140, 204, 205, 0, 1, 0, 0, 16, 0,   33,  230, 2, 0, 0, 0,
                        63, 12,  204, 205, 0, 1, 0, 0, 12, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 8, 0, 0,
                        19, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0, 0, 0, 0};
u8 gSharpClawCoDeflectHitboxFlags[24] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};
u8 gSharpClawAsMoveSelectTable[468] = {0,  0,   0,   0,   0, 0, 0, 0,  21, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0,  0, 0, 0,
                        36, 0,   0,   0,   0, 0, 0, 0,  63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1,  0, 0, 0,
                        63, 166, 102, 102, 0, 0, 0, 0,  25, 0,   9,   230, 1, 0, 0, 0, 63, 128, 0,   0,   0,  1, 0, 0,
                        16, 0,   33,  230, 2, 0, 0, 0,  63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1,  0, 0, 0,
                        63, 128, 0,   0,   0, 1, 0, 0,  16, 0,   33,  230, 2, 0, 0, 0, 63, 38,  102, 102, 0,  8, 0, 0,
                        19, 0,   0,   0,   0, 0, 0, 0,  63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1,  0, 0, 0,
                        63, 128, 0,   0,   0, 0, 0, 0,  24, 0,   5,   230, 1, 0, 0, 0, 63, 38,  102, 102, 0,  4, 0, 0,
                        17, 0,   0,   0,   0, 0, 0, 0,  63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1,  0, 0, 0,
                        63, 38,  102, 102, 0, 2, 0, 0,  18, 0,   0,   0,   0, 0, 0, 0, 63, 128, 0,   0,   0,  0, 0, 0,
                        2,  0,   0,   0,   0, 0, 0, 0,  63, 166, 102, 102, 0, 0, 0, 0, 25, 0,   9,   230, 1,  0, 0, 0,
                        63, 128, 0,   0,   0, 1, 0, 0,  16, 0,   33,  230, 2, 0, 0, 0, 63, 128, 0,   0,   0,  0, 0, 0,
                        24, 0,   5,   230, 1, 0, 0, 0,  63, 128, 0,   0,   0, 1, 0, 0, 16, 0,   33,  230, 2,  0, 0, 0,
                        63, 38,  102, 102, 0, 8, 0, 0,  19, 0,   0,   0,   0, 0, 0, 0, 63, 166, 102, 102, 0,  0, 0, 0,
                        25, 0,   9,   230, 1, 0, 0, 0,  63, 128, 0,   0,   0, 0, 0, 0, 24, 0,   5,   230, 1,  0, 0, 0,
                        63, 38,  102, 102, 0, 2, 0, 0,  18, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0,  4, 0, 0,
                        17, 0,   0,   0,   0, 0, 0, 0,  63, 140, 204, 205, 0, 1, 0, 0, 16, 0,   33,  230, 2,  0, 0, 0,
                        63, 12,  204, 205, 0, 1, 0, 0,  12, 0,   0,   0,   0, 0, 0, 0, 63, 38,  102, 102, 0,  8, 0, 0,
                        19, 0,   0,   0,   0, 0, 0, 0,  63, 38,  102, 102, 0, 2, 0, 0, 18, 0,   0,   0,   0,  0, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 11, 24, 1,   0,   0,   0, 0, 0, 0, 0,  0,   0,   12,  25, 1, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 10, 16, 2,   0,   0};
u8 gSharpClawAsDeflectHitboxFlags[24] = {0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0};
u8 gBossGeneralLocomotionMoves[48] = {60, 35, 215, 10, 0, 0, 0, 0, 0, 0, 0, 0, 60, 35, 215, 10, 0, 0, 0, 0, 1, 0, 0, 0,
                       60, 35, 215, 10, 0, 0, 0, 0, 2, 0, 0, 0, 60, 35, 215, 10, 0, 0, 0, 0, 1, 0, 0, 0};
u8 gBossGeneralIdleMoveTable[24] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 64, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
u8 gBossGeneralAnimEventMoves[300] = {
    63, 128, 0,  0, 0, 0, 0,  11,  20, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  16, 0, 0, 0, 63, 128, 0,  0, 0, 0,
    0,  11,  18, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  18, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  19, 0, 0, 0,
    63, 128, 0,  0, 0, 0, 0,  11,  19, 0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0,
    0,  0,   0,  0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  20, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  17, 0, 0, 0,
    63, 128, 0,  0, 0, 0, 0,  11,  18, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  18, 0, 0, 0, 63, 128, 0,  0, 0, 0,
    0,  11,  19, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  19, 0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0,
    0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  20, 0, 0, 0, 63, 128, 0,  0, 0, 0,
    0,  11,  16, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  18, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  18, 0, 0, 0,
    63, 128, 0,  0, 0, 0, 0,  11,  19, 0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  19, 0, 0, 0, 0,  0,   0,  0, 0, 0,
    0,  11,  0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 0,  0,   0,  0, 0, 0, 63, 128, 0,  0, 0, 0, 0,  11,  20, 0, 0, 0};
u8 gBossGeneralBlockReactionMoves[300] = {62, 148, 122, 225, 0, 0, 0, 11, 20, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 16, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 18, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 18, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 19, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 19, 2, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 20, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 17, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 18, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 18, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 19, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 19, 2, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 20, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 16, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 18, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 18, 2, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 19, 2, 0, 0, 62, 148, 122, 225, 0, 0, 0, 11, 19, 2, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0,  0,  0, 0, 0,
                        62, 148, 122, 225, 0, 0, 0, 11, 20, 2, 0, 0};
u8 gBossGeneralAttackHitVolumes[36] = {0, 0,  0, 0, 0, 0, 0, 11, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0,
                       0, 12, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0, 0, 10, 0, 1, 0, 0};
u8 gBossGeneralModeIdleMoves[96] = {63, 128, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 63, 128, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0,
                       63, 0,   0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0,  0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                       0,  0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 63, 0,   0, 0, 0, 0, 0, 0, 7, 0, 0, 0,
                       63, 128, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 63, 128, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0};
u8 gBossGeneralHitReactionMoves[300] = {63, 0,   0,   0,   0, 0, 0, 0, 15, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 12, 0, 0, 0,
                        63, 76,  204, 205, 0, 0, 0, 0, 14, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 14, 0, 0, 0,
                        63, 76,  204, 205, 0, 0, 0, 0, 13, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 13, 0, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0,
                        63, 192, 0,   0,   0, 0, 0, 0, 15, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 12, 0, 0, 0,
                        64, 0,   0,   0,   0, 0, 0, 0, 14, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 14, 0, 0, 0,
                        63, 0,   0,   0,   0, 0, 0, 0, 13, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 13, 0, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0,
                        63, 76,  204, 205, 0, 0, 0, 0, 15, 0, 0, 0, 63, 0,   0,   0,   0, 0, 0, 0, 12, 0, 0, 0,
                        63, 153, 153, 154, 0, 0, 0, 0, 14, 0, 0, 0, 63, 153, 153, 154, 0, 0, 0, 0, 14, 0, 0, 0,
                        63, 153, 153, 154, 0, 0, 0, 0, 13, 0, 0, 0, 63, 153, 153, 154, 0, 0, 0, 0, 13, 0, 0, 0,
                        0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0, 0,  0,   0,   0,   0, 0, 0, 0, 0,  0, 0, 0,
                        63, 192, 0,   0,   0, 0, 0, 0, 15, 0, 0, 0};
u8 gBossGeneralNullMoveChain[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
u8 gBossGeneralMoveSelectTable[240] = {
    0,  0,   0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 64, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0,  0,   0, 0, 0, 0,
    63, 128, 0, 0, 0, 8, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 63, 128, 0, 0, 0, 1, 0, 0, 1, 0, 0,  0,   0, 0, 0, 0,
    64, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 63, 128, 0, 0, 0, 4, 0, 0, 3, 0, 0,  0,   0, 0, 0, 0,
    63, 128, 0, 0, 0, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 63, 128, 0, 0, 0, 8, 0, 0, 5, 0, 0,  0,   0, 0, 0, 0,
    64, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 63, 128, 0, 0, 0, 2, 0, 0, 6, 0, 0,  0,   0, 0, 0, 0,
    63, 128, 0, 0, 0, 4, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 63, 128, 0, 0, 0, 1, 0, 0, 1, 0, 33, 230, 2, 0, 0, 0,
    63, 128, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 63, 128, 0, 0, 0, 8, 0, 0, 5, 0, 0,  0,   0, 0, 0, 0,
    63, 128, 0, 0, 0, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0};
u8 gBossGeneralDeflectHitboxFlags[24] = {0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
FamilyTable gBaddieFamilyTables[6] = {
    {gSharpClawLocomotionMoves, gSharpClawIdleMoveTable, gSharpClawAnimEventMoves, gSharpClawMoveSelectTable, gSharpClawHitReactionMoves, gSharpClawAttackHitVolumes, gSharpClawModeIdleMoves, gSharpClawReactionMoveChain,
     gSharpClawDeflectHitboxFlags, gSharpClawBlockReactionMoves},
    {gSharpClawLocomotionMoves, gSharpClawIdleMoveTable, gSharpClawAnimEventMoves, gSharpClawMoveSelectTable, gSharpClawHitReactionMoves, gSharpClawAttackHitVolumes, gSharpClawModeIdleMoves, gSharpClawReactionMoveChain,
     gSharpClawDeflectHitboxFlags, gSharpClawBlockReactionMoves},
    {gSharpClawLocomotionMoves, gSharpClawIdleMoveTable, gSharpClawAnimEventMoves, gSharpClawSoMoveSelectTable, gSharpClawHitReactionMoves, gSharpClawAttackHitVolumes, gSharpClawModeIdleMoves, gSharpClawReactionMoveChain,
     gSharpClawSoDeflectHitboxFlags, gSharpClawBlockReactionMoves},
    {gSharpClawLocomotionMoves, gSharpClawIdleMoveTable, gSharpClawAnimEventMoves, gSharpClawCoMoveSelectTable, gSharpClawHitReactionMoves, gSharpClawCoAttackHitVolumes, gSharpClawModeIdleMoves, gSharpClawReactionMoveChain,
     gSharpClawCoDeflectHitboxFlags, gSharpClawBlockReactionMoves},
    {gSharpClawLocomotionMoves, gSharpClawIdleMoveTable, gSharpClawAnimEventMoves, gSharpClawAsMoveSelectTable, gSharpClawHitReactionMoves, gSharpClawAttackHitVolumes, gSharpClawModeIdleMoves, gSharpClawReactionMoveChain,
     gSharpClawAsDeflectHitboxFlags, gSharpClawBlockReactionMoves},
    {gBossGeneralLocomotionMoves, gBossGeneralIdleMoveTable, gBossGeneralAnimEventMoves, gBossGeneralMoveSelectTable, gBossGeneralHitReactionMoves, gBossGeneralAttackHitVolumes, gBossGeneralModeIdleMoves, gBossGeneralNullMoveChain,
     gBossGeneralDeflectHitboxFlags, gBossGeneralBlockReactionMoves},
};
u8 gBaddieEventDelayRanges[6][2] = {{15, 60}, {10, 50}, {7, 20}, {5, 20}, {3, 15}, {3, 15}};
f32 gBaddieFamilySpeedScales[6] = {0.5f, 0.5f, 0.7f, 0.6f, 1.5f, 1.5f};
u32 gGroundBaddieModelChainIds[4] = {6, 7, 8, 9};

void battleDroidUpdateWhileFrozen(int obj, u8* state, int arg, int code, int wpad0, int wpad1, Vec* wpad2, int wpad3)
{
    if (code == 0x10)
    {
        ((EnemyState*)state)->flags2E8 |= 0x20;
    }
    else
    {
        ((EnemyState*)state)->flags2E8 |= 0x8;
    }
}

void battleDroidUpdate(int obj, int state)
{
}

void battleDroidUpdateAttack(int obj, int state)
{
    f32* pos = (f32*)((EnemyState*)state)->trackedObj;
    baddieTurnTowardPoint((GameObject*)obj, state, pos[3], pos[5], 0xf, 0);
}

void battleDroidInit(int unused, char* p)
{
    f32 v1c;
    ((EnemyState*)p)->sightRange = 60.0f;
    ((EnemyState*)p)->flags2E4 = 1;
    ((EnemyState*)p)->flags2E4 |= 0x80;
    ((EnemyState*)p)->animPlaySpeed = 0.005f;
    ((EnemyState*)p)->gravity = 0.17f;
    ((EnemyState*)p)->drag = 0.97f;
    ((EnemyState*)p)->moveId0 = 0;
    v1c = 3.0f;
    ((EnemyState*)p)->moveSpeedScale0 = v1c;
    ((EnemyState*)p)->moveId1 = 0;
    ((EnemyState*)p)->moveSpeedScale1 = 1.25f;
    ((EnemyState*)p)->moveId2 = 0;
    ((EnemyState*)p)->moveSpeedScale2 = v1c;
}

#define WISPBADDIE_OBJFLAG_PARENT_SLACK 0x1000

typedef struct WispEventRow
{
    f32 blend; /* +0x0 */
    u32 flags; /* +0x4 (low byte = move flags) */
    u8 moveId; /* +0x8 */
    u8 pad9[3];
} WispEventRow;
STATIC_ASSERT(sizeof(WispEventRow) == 0xc);
STATIC_ASSERT(offsetof(WispEventRow, moveId) == 0x8);

u32 wispBaddieProcessAnimEvent(GameObject* obj, u8* state, u32 allowNewEvent)
{
    u8* base = gBaddieMoveProgressTable;
    u8* sequenceBase;
    WispEventRow* eventRows;
    u8 eventIndex;
    int ei;
    int flag20;
    u32 stateFlags;
    u8 eventFlags;
    u8 sequenceIndex;
    f32 blendScale;
    f32 blendTimer;
    int eventTableIndex;
    WispEventRow* row;
    u32 sf2;

    sequenceIndex = ((EnemyState*)state)->userData2;
    sequenceBase = base + sequenceIndex * 0x28;
    eventRows = *(WispEventRow**)(sequenceBase + 0x1444);
    stateFlags = ((EnemyState*)state)->controlFlags;
    if ((stateFlags & 0x4000) != 0)
    {
        return 0;
    }
    if (((EnemyState*)state)->wisp.unk328 && ((EnemyState*)state)->phaseAngle != 0)
    {
        return 0;
    }
    eventFlags = ((EnemyState*)state)->flags2F1;
    ei = eventFlags & 0x1f;
    eventIndex = ei;
    if ((ei & 0x10) != 0)
    {
        eventIndex = ei & ~0x8;
    }
    if (eventIndex > 0x18)
    {
        eventIndex = 0;
    }
    flag20 = eventFlags & 0x20;
    if (flag20 != 0)
    {
        blendScale = 3.0f;
        eventIndex = 0;
    }
    else
    {
        blendScale = 1.0f;
    }
    if ((u8)allowNewEvent != 0)
    {
        if ((eventFlags != 0 || ((EnemyState*)state)->wisp.eventDelayTimer) &&
            (stateFlags & 0x40) == 0 && flag20 == 0)
        {
            if (((EnemyState*)state)->wisp.eventDelayTimer)
            {
                ((EnemyState*)state)->wisp.eventDelayTimer = ((EnemyState*)state)->wisp.eventDelayTimer - timeDelta;
                if (((EnemyState*)state)->wisp.eventDelayTimer <= 0.0f)
                {
                    ((EnemyState*)state)->wisp.eventDelayTimer = 0.0f;
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                eventTableIndex = sequenceIndex * 2;
                ((EnemyState*)state)->wisp.eventDelayTimer =
                    ((EnemyState*)state)->intervalTimer +
                    (f32)(int)randomGetRange(base[eventTableIndex + 0x152c], base[eventTableIndex + 0x152d]);
                ((EnemyState*)state)->intervalTimer = 0.0f;
                return 0;
            }
        }
    }
    if ((((u8)allowNewEvent != 0 && ((EnemyState*)state)->flags2F1 != 0 && eventRows[eventIndex].moveId != 0) ||
         (((EnemyState*)state)->flags2F1 & 0x20) != 0) &&
        !(((EnemyState*)state)->familyData.wisp.activeEventIndex == eventIndex && 0.0f != ((EnemyState*)state)->wisp.moveHoldTimer))
    {
        sf2 = ((EnemyState*)state)->controlFlags;
        if ((sf2 & 0x800080) != 0 || (((EnemyState*)state)->flags2F1 & 0x20) != 0)
        {
            blendTimer = 60.0f * (blendScale * (row = &eventRows[eventIndex])->blend);
            ((EnemyState*)state)->wisp.unk330 = blendTimer;
            ((EnemyState*)state)->wisp.moveHoldTimer = blendTimer;
            ((EnemyState*)state)->controlFlags = ((EnemyState*)state)->controlFlags | 0x40;
            ((EnemyState*)state)->curveIndex = ((EnemyState*)state)->curveIndex | 0x80;
            ((EnemyState*)state)->curveParamA = 0;
            ((EnemyState*)state)->curveParamB = 0;
            Baddie_SetMove(obj, state, row->moveId, blendScale * row->blend, 0, row->flags & 0xff);
            ObjAnim_SetMoveProgress(&obj->anim, *(f32*)(base + row->moveId * 4));
            ((EnemyState*)state)->familyData.wisp.activeEventIndex = eventIndex;
            return 1;
        }
        if ((sf2 & 0x40000000) != 0)
        {
            groundBaddiePickNextMove(obj, (u8*)state);
        }
        return 0;
    }
    if (((EnemyState*)state)->wisp.moveHoldTimer)
    {
        GameObject* pos = (GameObject*)((EnemyState*)state)->trackedObj;
        baddieTurnTowardPoint(obj, (int)state, pos->anim.localPosX, pos->anim.localPosZ, 0xf, 0);
        if (((EnemyState*)state)->animPlaySpeed > 0.0166f)
        {
            ((EnemyState*)state)->animPlaySpeed = ((EnemyState*)state)->animPlaySpeed - 0.005f;
        }
        if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            eventTableIndex = ((EnemyState*)state)->familyData.wisp.activeEventIndex;
            Baddie_SetMove(obj, state, eventRows[eventTableIndex].moveId,
                           eventRows[((EnemyState*)state)->familyData.wisp.activeEventIndex].blend, 0,
                           eventRows[eventTableIndex].flags & 0xff);
            ObjAnim_SetMoveProgress(
                &obj->anim, *(f32*)(base + eventRows[((EnemyState*)state)->familyData.wisp.activeEventIndex].moveId * 4));
        }
        ((EnemyState*)state)->wisp.moveHoldTimer = ((EnemyState*)state)->wisp.moveHoldTimer - timeDelta;
        if (((EnemyState*)state)->wisp.moveHoldTimer <= 0.0f)
        {
            ((EnemyState*)state)->wisp.moveHoldTimer = 0.0f;
            ((EnemyState*)state)->controlFlags &= ~0x40LL;
            ((EnemyState*)state)->controlFlags =
                ((EnemyState*)state)->controlFlags | (u64)BADDIE_CONTROL_SEQUENCE_DRIVEN;
            ((EnemyState*)state)->curveIndex = ((EnemyState*)state)->curveIndex & ~0x80;
            ((EnemyState*)state)->familyData.wisp.activeEventIndex = 0;
            return 0;
        }
        else
        {
            return 1;
        }
    }
    return 0;
}

void wispBaddiePlayMoveEventSfx(GameObject* obj, void* animState)
{
    GameObject* player;
    f32 distance;
    f32 rumbleFalloff;

    if ((((EnemyState*)animState)->animEventMask & 0x200) != 0)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_sml_trex_snap3);
        player = Obj_GetPlayerObject();
        if ((player->objectFlags & WISPBADDIE_OBJFLAG_PARENT_SLACK) == 0)
        {
            distance = Vec_distance(&(obj)->anim.worldPosX, &player->anim.worldPosX);
            if (distance <= 640.0f)
            {
                rumbleFalloff = 1.0f - distance / 640.0f;
                rumbleFalloff = 3.0f * rumbleFalloff;
                doRumble(rumbleFalloff);
            }
            CameraShake_ApplyRadial((obj)->anim.localPosX, (obj)->anim.localPosY, (obj)->anim.localPosZ, 640.0f,
                                    4.0f);
        }
    }
    if ((((EnemyState*)animState)->animEventMask & 0x40) != 0)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_spotfox01);
    }
    if ((((EnemyState*)animState)->animEventMask & 0x1000) != 0)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_scream1);
    }
    if ((((EnemyState*)animState)->animEventMask & 1) != 0)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_pullup2);
    }
    if ((((EnemyState*)animState)->animEventMask & 0x80) != 0)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_death01);
    }
}

void wispBaddieQueueNextEvent(int obj, int delta)
{
    u8* inner = ((GameObject*)obj)->extra;
    u8* ptr = gBaddieFamilyTables[inner[0x33b]].tbl4;
    inner[0x33d] = (u8)(delta + (u32)ptr[8] + 1);
    inner[0x33e] = 1;
}

/* explosive-barrel attackers; a hit from one skips the sword/impact sfx.
   retail OBJECTS.bin names "GunPowderBa" and "MetalBarrel" (both DLL 0x158) */
#define NEWSEQOBJ_ATTACKER_GUNPOWDERBARREL 0x6d
#define NEWSEQOBJ_ATTACKER_METALBARREL     0x754

/* per-family anim-table row: speed + flags + anim ids and chain links */
typedef struct
{
    f32 speed; /* 0x0 */
    u32 flags; /* 0x4 */
    u8 anim;   /* 0x8 */
    u8 next;   /* 0x9 */
    u8 alt;    /* 0xa */
    u8 padB;   /* 0xb */
    u32 extra; /* 0xc */
} SeqRow16;

typedef struct
{
    f32 speed; /* 0x0 */
    u32 flags; /* 0x4 */
    u8 anim;   /* 0x8 */
    u8 next;   /* 0x9 */
    u16 padA;  /* 0xa */
} IdleRow;

u8 sharpClawHandleHitMessage(GameObject* obj, u8* state, GameObject* attacker, int msgId, int arrIdx, int damage,
                                Vec* hitPos, int sector, f32 hDist, f32 vDist)
{
    u8* animRows;
    u8* rowsC;
    u8* rowsB;
    u8* trig;
    u8 ret;

    animRows = gBaddieFamilyTables[((EnemyState*)state)->userData2].tbl10;
    rowsC = gBaddieFamilyTables[((EnemyState*)state)->userData2].tbl24;
    rowsB = gBaddieFamilyTables[((EnemyState*)state)->userData2].tbl1c;
    trig = gBaddieFamilyTables[((EnemyState*)state)->userData2].tbl20;
    ret = 0;

    if (((EnemyState*)state)->userData2 == 5)
    {
        ((EnemyState*)state)->flags2E8 |= 0x10;
        return 0;
    }
    if (msgId == 0xe)
    {
        damage = damage * 0xa;
    }
    if (obj->anim.currentMove == animRows[0x128])
    {
        return 0;
    }
    if (msgId == 0x10)
    {
        ((EnemyState*)state)->flags2E8 |= 0x28;
        return 0;
    }
    if ((((EnemyState*)state)->controlFlags & 0x40) != 0 ||
        (trig[arrIdx] != 0 && ((u32)(msgId - 0xe) <= 1 || msgId == 0x13)))
    {
        if (msgId != 0x11)
        {
            f32 z;
            if (msgId != 0x1a && attacker->anim.romDefNo != NEWSEQOBJ_ATTACKER_GUNPOWDERBARREL && attacker->anim.romDefNo != NEWSEQOBJ_ATTACKER_METALBARREL)
            {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_swdout1);
                Sfx_PlayFromObject((u32)obj, SFXTRIG_gethit02);
            }
            ((EnemyState*)state)->flags2E8 |= 0x10;
            {
                IdleRow* rows = (IdleRow*)rowsC;
                Baddie_SetMove(obj, state, rows[state[0x33c]].anim, *(f32*)(rowsC + state[0x33c] * 12), 0,
                               (u8)rows[state[0x33c]].flags);
            }
            ObjAnim_SetMoveProgress(&obj->anim,
                                    *(f32*)(gBaddieMoveProgressTable + rowsC[state[0x33c] * 12 + 8] * 4));
            if (rowsC[state[0x33c] * 12 + 0xa] != 0)
            {
                state[0x33a] = rowsC[state[0x33c] * 12 + 0xa];
            }
            ret = rowsC[state[0x33c] * 12 + 9];
            ((EnemyState*)state)->seqObj.unk32C = ((EnemyState*)state)->seqObj.unk330;
            z = 0.0f;
            ((EnemyState*)state)->seqObj.unk324 = z;
            ((EnemyState*)state)->intervalTimer = z;
        }
    }
    else
    {
        u32 amount;
        f32 z;

        if (msgId == 0x11)
        {
            amount = 0x18;
        }
        else
        {
            amount = state[0x2f1] & 0x1f;
            if ((u32)(state[0x2f1] & 0x1f) > 0x18)
            {
                amount = 0;
            }
        }
        z = 0.0f;
        ((EnemyState*)state)->seqObj.unk324 = z;
        if (state[0x2f1] & 0x18)
        {
            if (state[0x2f1] & 1)
            {
                ((EnemyState*)state)->intervalTimer = 50.0f;
            }
            else
            {
                ((EnemyState*)state)->intervalTimer = 30.0f;
            }
        }
        else
        {
            ((EnemyState*)state)->intervalTimer = z;
        }
        if (((EnemyState*)state)->seqObj.seqTimer && ((EnemyState*)state)->phaseAngle != 0)
        {
            {
                SeqRow16* rows = (SeqRow16*)rowsB;
                Baddie_SetMove(obj, state, rows[rowsB[((EnemyState*)state)->phaseAngle * 16 + 0xb]].anim,
                               *(f32*)(rowsB + rowsB[((EnemyState*)state)->phaseAngle * 16 + 0xb] * 16), 0,
                               (u8)rows[rowsB[((EnemyState*)state)->phaseAngle * 16 + 0xb]].flags);
            }
            ObjAnim_SetMoveProgress(
                &obj->anim,
                *(f32*)(gBaddieMoveProgressTable + rowsB[rowsB[((EnemyState*)state)->phaseAngle * 16 + 0xb] * 16 + 8] * 4));
        }
        else
        {
            int off = (u8)amount * 12;
            IdleRow* rows = (IdleRow*)animRows;

            Baddie_SetMove(obj, state, rows[(u8)amount].anim, *(f32*)(animRows + (u8)amount * 12), 0,
                           (u8)rows[(u8)amount].flags);
            ObjAnim_SetMoveProgress(&obj->anim,
                                    *(f32*)(gBaddieMoveProgressTable + rows[(u8)amount].anim * 4));
            ((EnemyState*)state)->phaseAngle = animRows[off + 9];
            ((EnemyState*)state)->seqObj.seqTimer = (f32)(u32) * (u16*)(state + 0x2ec);
        }
        ((EnemyState*)state)->flags2E8 |= 8;
        if (attacker->anim.classId == 0x1c)
        {
            return 0;
        }
        {
            GameObject* other = attacker->ownerObj;
            if (other != 0 && other->anim.classId == 0x1c)
            {
                return 0;
            }
        }
        if (state[0x2f1] & 0x10)
        {
            damage = 0x14;
        }
        else
        {
            state[0x2f5] = 0;
        }
        if (damage > ((EnemyState*)state)->current)
        {
            ((EnemyState*)state)->current = 0;
        }
        else
        {
            ((EnemyState*)state)->current = ((EnemyState*)state)->current - damage;
        }
        if (((EnemyState*)state)->current == 0)
        {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_land);
        }
        else
        {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_attack);
        }
        if (msgId != 0x1a && msgId != 0x1f && attacker->anim.romDefNo != NEWSEQOBJ_ATTACKER_GUNPOWDERBARREL && attacker->anim.romDefNo != NEWSEQOBJ_ATTACKER_METALBARREL)
        {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_stftest);
        }
    }
    return ret;
}

/* sidekick-toy main update: timer-driven 16-stride anim chain, curve chase
 * with speed/turn shaping, idle anims. */

void sharpClawUpdateIdle(GameObject* obj, u8* state)
{
    RomCurveWalker* path = *(RomCurveWalker**)state;
    u8* tbl4;
    u8* tbl0;
    u8* tbl1c;
    u32 flags;

    tbl4 = gBaddieFamilyTables[((EnemyState*)state)->userData2].tbl4;
    tbl0 = gBaddieFamilyTables[((EnemyState*)state)->userData2].tbl0;
    tbl1c = gBaddieFamilyTables[((EnemyState*)state)->userData2].tbl1c;

    if (((EnemyState*)state)->userData2 == 5 && (((EnemyState*)state)->controlFlags & 0x800000))
    {
        mainSetBits(GAMEBIT_BaddieRelated1C8, 1);
    }
    wispBaddiePlayMoveEventSfx(obj, state);
    {
        f32 t = ((EnemyState*)state)->seqObj.seqTimer;
        f32 z = 0.0f;
        if (t != z && ((EnemyState*)state)->phaseAngle != 0)
        {
            ((EnemyState*)state)->seqObj.seqTimer = t - timeDelta;
            if (((EnemyState*)state)->seqObj.seqTimer <= z)
            {
                ((EnemyState*)state)->seqObj.seqTimer = z;
                ((EnemyState*)state)->controlFlags |= (u64)BADDIE_CONTROL_SEQUENCE_DRIVEN;
                ((EnemyState*)state)->phaseAngle = tbl1c[((EnemyState*)state)->phaseAngle * 16 + 0xa];
            }
        }
    }
    if ((u8)wispBaddieProcessAnimEvent(obj, state, 0) != 0)
    {
        return;
    }
    if (((EnemyState*)state)->familyData.sharpClaw.idleRow != 0)
    {
        if (((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN)
        {
            f32 z = 0.0f;
            (obj)->anim.velocityZ = z;
            (obj)->anim.velocityY = z;
            (obj)->anim.velocityX = z;
            {
                IdleRow* idleRows = (IdleRow*)tbl4;
                Baddie_SetMove(obj, state, idleRows[((EnemyState*)state)->familyData.sharpClaw.idleRow].anim, *(f32*)(tbl4 + ((EnemyState*)state)->familyData.sharpClaw.idleRow * 12), 0,
                               (u8)idleRows[((EnemyState*)state)->familyData.sharpClaw.idleRow].flags);
            }
            ObjAnim_SetMoveProgress(&obj->anim,
                                    *(f32*)(gBaddieMoveProgressTable + tbl4[((EnemyState*)state)->familyData.sharpClaw.idleRow * 12 + 8] * 4));
            ((EnemyState*)state)->familyData.sharpClaw.idleRow = tbl4[((EnemyState*)state)->familyData.sharpClaw.idleRow * 12 + 9];
            ((EnemyState*)state)->familyData.sharpClaw.idleRowStarted = 0;
        }
        if (((EnemyState*)state)->familyData.sharpClaw.idleRowStarted == 0)
        {
            return;
        }
    }
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_JUST_TRIGGERED) && ((EnemyState*)state)->familyData.sharpClaw.idleRow == 0)
    {
        sidekickToy_updateCurveTargetLatch(obj);
    }
    flags = ((EnemyState*)state)->controlFlags;
    if (flags & BADDIE_CONTROL_PATH_FOLLOW)
    {
        f32 dist;
        f32 delta;

        {
            f32 dx = path->posX - (obj)->anim.localPosX;
            f32 dz = path->posZ - (obj)->anim.localPosZ;
            dist = sqrtf(dx * dx + dz * dz);
        }
        if (dist > 64.0f)
        {
            dist = 64.0f;
        }
        {
            f32 diff = 64.0f - dist;
            f32 spd = diff / 64.0f;
            *(f32*)(state + 0x310) = spd * ((EnemyState*)state)->pathStep;
        }
        if (*(f32*)(state + 0x310) < 0.25f)
        {
            *(f32*)(state + 0x310) = 0.25f;
        }
        if (Curve_AdvanceAlongPath(&path->curve, *(f32*)(state + 0x310)) != 0 || path->atSegmentEnd != 0)
        {
            if ((*gRomCurveInterface)->goNextPoint(path) != 0)
            {
                sidekickToy_updateCurveTargetLatch(obj);
            }
        }
        delta = (f32)(int)((u16)getAngle(path->tangentX, path->tangentZ) + 0x8000 - (u16)(obj)->anim.rotX);
        if (delta > 32768.0f)
        {
            delta = -65535.0f + delta;
        }
        if (delta < -32768.0f)
        {
            delta = 65535.0f + delta;
        }
        ((EnemyState*)state)->animPlaySpeed =
            (((EnemyState*)state)->pathStep - *(f32*)(state + 0x310)) / 60.0f *
            (1.0f - ((delta >= 0.0f) ? delta : -delta) / 65535.0f);
        if (*(f32*)(state + 0x308) < 0.005f)
        {
            *(f32*)(state + 0x308) = 0.005f;
        }
        if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) && ((EnemyState*)state)->familyData.sharpClaw.idleRow == 0)
        {
            if (((EnemyState*)state)->phaseAngle != 0)
            {
                SeqRow16* seqRow16 = (SeqRow16*)tbl1c;
                Baddie_SetMove(obj, state, seqRow16[((EnemyState*)state)->phaseAngle].anim,
                               *(f32*)(tbl1c + ((EnemyState*)state)->phaseAngle * 16), 0,
                               (u8)seqRow16[((EnemyState*)state)->phaseAngle].flags);
                ObjAnim_SetMoveProgress(
                    &obj->anim,
                    *(f32*)(gBaddieMoveProgressTable + tbl1c[((EnemyState*)state)->phaseAngle * 16 + 8] * 4));
                ((EnemyState*)state)->phaseAngle = tbl1c[((EnemyState*)state)->phaseAngle * 16 + 9];
            }
            else if (*(f32*)(state + 0x310) > 0.0001f)
            {
                ((EnemyState*)state)->curveIndex = 0;
                ((EnemyState*)state)->curveParamA = 0;
                ((EnemyState*)state)->curveParamB = 0;
                if (*(f32*)(state + 0x310) > 1.2f)
                {
                    ((EnemyState*)state)->rootMotionFlags = 1;
                    ObjAnim_SetCurrentMove((int)obj, tbl0[0x20], 0.0f, 0);
                }
                else
                {
                    ((EnemyState*)state)->rootMotionFlags = 1;
                    ObjAnim_SetCurrentMove((int)obj, tbl0[0x14], 0.0f, 0);
                }
            }
            else
            {
                ((EnemyState*)state)->curveIndex = 0;
                ((EnemyState*)state)->curveParamA = 0;
                ((EnemyState*)state)->curveParamB = 0;
                ((EnemyState*)state)->rootMotionFlags = 1;
                *(f32*)(state + 0x308) = 0.01f;
                ObjAnim_SetCurrentMove((int)obj, tbl0[8], 0.0f, 0);
                *(f32*)(state + 0x310) = 0.0f;
            }
        }
        baddieTurnTowardPoint(obj, (int)state, path->posX, path->posZ, 0xf, 0);
    }
    else
    {
        if (((EnemyState*)state)->familyData.sharpClaw.idleRow == 0 && (flags & BADDIE_CONTROL_SEQUENCE_DRIVEN))
        {
            u8 r = randomGetRange(1, tbl4[8]);
            if (((EnemyState*)state)->phaseAngle != 0)
            {
                {
                    SeqRow16* seqRow16 = (SeqRow16*)tbl1c;
                    ((EnemyState*)state)->curveIndex = (u8)seqRow16[((EnemyState*)state)->phaseAngle].extra;
                    Baddie_SetMove(obj, state, seqRow16[((EnemyState*)state)->phaseAngle].anim,
                                   *(f32*)(tbl1c + ((EnemyState*)state)->phaseAngle * 16), 0,
                                   (u8)seqRow16[((EnemyState*)state)->phaseAngle].flags);
                }
                ObjAnim_SetMoveProgress(
                    &obj->anim,
                    *(f32*)(gBaddieMoveProgressTable + tbl1c[((EnemyState*)state)->phaseAngle * 16 + 8] * 4));
                ((EnemyState*)state)->phaseAngle = tbl1c[((EnemyState*)state)->phaseAngle * 16 + 9];
            }
            else
            {
                int off;
                IdleRow* row;
                if ((obj)->anim.currentMove != (r = (row = (IdleRow*)(tbl4 + (off = r * 12)))->anim) ||
                    r != 0)
                {
                    ((EnemyState*)state)->curveIndex = 0;
                    ((EnemyState*)state)->curveParamA = 0;
                    ((EnemyState*)state)->curveParamB = 0;
                    Baddie_SetMove(obj, state, row->anim, *(f32*)(tbl4 + off), 0, 3);
                    ObjAnim_SetMoveProgress(&obj->anim,
                                            *(f32*)(gBaddieMoveProgressTable + row->anim * 4));
                }
            }
        }
    }
}

/* sidekick-toy anim-chain advance: timer-driven 16-stride SeqRow16 chain +
 * curve-follow speed shaping, called from the sharpClawUpdateIdle update path. */
void sharpClawUpdateApproach(GameObject* obj, void* state)
{
    u8* table = gBaddieMoveProgressTable;
    u8 idx = ((EnemyState*)state)->userData2;
    void* animCtrl = *(void**)(table + idx * 0x28 + 0x143c);
    IdleRow* idleSrc = (IdleRow*)(*(void**)(table + idx * 0x28 + 0x1454));
    u8* seqRows = *(u8**)(table + idx * 0x28 + 0x1458);

    if (idx == 5 && (((EnemyState*)state)->controlFlags & 0x800000) != 0)
    {
        mainSetBits(GAMEBIT_BaddieRelated1C8, 1);
    }

    if (((EnemyState*)state)->trackedObj != NULL &&
        ((GameObject*)((EnemyState*)state)->trackedObj)->anim.classId == 1)
    {
        requestKrazoaShrineMusic();
    }

    wispBaddiePlayMoveEventSfx(obj, state);

    {
        if (((EnemyState*)state)->seqObj.seqTimer && ((EnemyState*)state)->phaseAngle != 0)
        {
            f32 zero = 0.0f;
            ((EnemyState*)state)->seqObj.seqTimer = ((EnemyState*)state)->seqObj.seqTimer - timeDelta;
            if (((EnemyState*)state)->seqObj.seqTimer <= zero)
            {
                ((EnemyState*)state)->seqObj.seqTimer = zero;
                ((EnemyState*)state)->controlFlags |= (u64)BADDIE_CONTROL_SEQUENCE_DRIVEN;
                {
                    SeqRow16* seqRow16 = (SeqRow16*)seqRows;
                    ((EnemyState*)state)->phaseAngle = seqRow16[((EnemyState*)state)->phaseAngle].alt;
                }
            }
        }
    }

    if ((u8)wispBaddieProcessAnimEvent(obj, state, 0) != 0)
    {
        return;
    }

    if ((((EnemyState*)state)->controlFlags & 0x20000000) != 0 && (((EnemyState*)state)->prevControlFlags & 0x20000000) == 0)
    {
        Sfx_PlayFromObject((u32)obj, SFXTRIG_sc_mumble02);
        ((EnemyState*)state)->controlFlags |= (u64)BADDIE_CONTROL_SEQUENCE_DRIVEN;
    }

    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        SeqRow16* seqRow16 = (SeqRow16*)seqRows;
        if (((EnemyState*)state)->phaseAngle != 0)
        {
            ((EnemyState*)state)->curveIndex = seqRow16[((EnemyState*)state)->phaseAngle].extra;
            Baddie_SetMove(obj, state, seqRow16[((EnemyState*)state)->phaseAngle].anim,
                           *(f32*)(seqRows + (((EnemyState*)state)->phaseAngle << 4)), 0,
                           (u8)seqRow16[((EnemyState*)state)->phaseAngle].flags);
            ObjAnim_SetMoveProgress(
                &obj->anim, *(f32*)(table + (seqRow16[((EnemyState*)state)->phaseAngle].anim << 2)));
            ((EnemyState*)state)->phaseAngle = seqRow16[((EnemyState*)state)->phaseAngle].next;
        }
        else
        {
            IdleRow* idleRows = idleSrc;
            u8 idleAnim;
            ((EnemyState*)state)->curveIndex = 0;
            ((EnemyState*)state)->curveParamA = 0;
            ((EnemyState*)state)->curveParamB = 0;
            idleAnim = idleRows[((EnemyState*)state)->turnOctant].anim;
            if (idleAnim == 0)
            {
                ((EnemyState*)state)->rootMotionFlags = 3;
                ObjAnim_SetCurrentMove((int)obj, *(u8*)((u8*)animCtrl + 0x2c), 0.0f, 0);
            }
            else
            {
                Baddie_SetMove(obj, state, idleAnim, idleRows[((EnemyState*)state)->turnOctant].speed, 0, 0xb);
                ObjAnim_SetMoveProgress(
                    &obj->anim, *(f32*)(table + (idleRows[((EnemyState*)state)->turnOctant].anim << 2)));
            }
        }
    }

    if ((s32)(obj)->anim.currentMove == *(u8*)((u8*)animCtrl + 0x2c))
    {
        ((EnemyState*)state)->animPlaySpeed =
            ((EnemyState*)state)->pathStep *
            (((f32)(u32)((EnemyState*)state)->targetDist / ((EnemyState*)state)->aggroRange / 60.0f) *
             ((f32*)(table + 0x1538))[((EnemyState*)state)->userData2]);
        if (((EnemyState*)state)->animPlaySpeed < 0.03f)
        {
            ((EnemyState*)state)->animPlaySpeed = 0.03f;
        }
    }

    if ((((EnemyState*)state)->rootMotionFlags & 8) == 0)
    {
        GameObject* tracked = (GameObject*)(((EnemyState*)state)->trackedObj);
        baddieTurnTowardPoint(obj, (int)state, tracked->anim.localPosX,
                    tracked->anim.localPosZ, 0xf, 0);
    }
}

typedef struct GroundBaddieModelChainDescriptor
{
    void* entries;
    s32 count;
} GroundBaddieModelChainDescriptor;

STATIC_ASSERT(sizeof(GroundBaddieModelChainDescriptor) == 8);

u8 gGroundBaddieAngleSectorOffsets[8] = {1, 4, 4, 3, 3, 5, 5, 1};
GroundBaddieModelChainDescriptor gGroundBaddieModelChainIdsDesc = {gGroundBaddieModelChainIds, 4};
GroundBaddieModelChainDescriptor gGroundBaddieModelChainDesc = {&gGroundBaddieModelChainIdsDesc, 0};
u16 gGroundBaddieTriggerResponseSeq[4] = {0x4FD, 0x4FE, 0x4FF, 0};

typedef struct
{
    f32 speed;
    u32 mask;
    u8 anim;
    u8 pad9;
    u8 r;
    u8 g;
    u8 b;
    u8 pad13[3];
} SeqEntry;

typedef struct
{
    u8 pad00[0x14];
    u8* hitEntries;
    u8 pad18[4];
    u8* sequenceEntries;
    u8 pad20[8];
} GroundBaddieSequenceTable;

/* Routines live in sibling baddie/seq TUs (fn_8014*, getAngle, math*,
   player*, hud, ObjModelChain). DAT_/lbl_/PTR_ are shared .data/.sdata
   tables and FP constants. */

EnemyTargetSearchResult gGroundBaddieTargetSearchResult[16];

void groundBaddiePickIdleMove(GameObject* obj, u8* state);

void groundBaddiePickIdleMove(GameObject* obj, u8* state)
{
    SeqEntry* entry;
    u32 idx;
    u8 wrapIdx;
    char* base;

    base = (char*)gBaddieFamilyTables;
    base += ((EnemyState*)state)->userData2 * 40;
    entry = *(SeqEntry**)(base + 12);
    if ((f32) * (u16*)(state + 0x2a4) > 0.6f * ((EnemyState*)state)->sightRange)
    {
        if ((f32) * (u16*)(state + 0x2a4) > 0.8f * ((EnemyState*)state)->sightRange)
        {
            state[0x33a] = (u8)(entry[0].anim + 2);
        }
        else
        {
            state[0x33a] = (u8)(entry[0].anim + 3);
        }
    }
    wrapIdx = 1;
    while (entry[idx = state[0x33a]].mask != 0 &&
           (((EnemyState*)state)->controlFlags & entry[idx].mask) == 0)
    {
        (((EnemyState*)state)->userData1)++;
        if (state[0x33a] > entry[0].anim)
        {
            state[0x33a] = wrapIdx;
        }
    }
    *(u8*)(state + 0x2f2) = entry[state[0x33a]].r;
    *(u8*)(state + 0x2f3) = entry[state[0x33a]].g;
    *(u8*)(state + 0x2f4) = entry[state[0x33a]].b;
    baddieSetMove(obj, (int)state, entry[state[0x33a]].anim, entry[state[0x33a]].speed, 0, 3);
    ObjAnim_SetMoveProgress(&obj->anim,
                            *(f32*)(gBaddieMoveProgressTable + entry[state[0x33a]].anim * 4));
    (((EnemyState*)state)->userData1)++;
    if (state[0x33a] > entry[0].anim)
    {
        state[0x33a] = 1;
    }
}

void groundBaddiePickNextMove(GameObject* obj, u8* state)
{
    SeqEntry* entry;
    u32 idx;
    s16 d;
    char* base;

    base = (char*)gBaddieFamilyTables;
    base += ((EnemyState*)state)->userData2 * 40;
    entry = *(SeqEntry**)(base + 12);
    if (enemy_findNearbyEnemies(obj, 100.0f, 1, 16, gGroundBaddieTargetSearchResult) >= 1)
    {
        if (gGroundBaddieTargetSearchResult[0].dist <= 40 && *(u16*)(state + 0x2a0) != 3 &&
            *(u16*)(state + 0x2a0) != 4)
        {
            d = getAngle(obj->anim.localPosX - gGroundBaddieTargetSearchResult[0].obj->anim.localPosX,
                         obj->anim.localPosZ - gGroundBaddieTargetSearchResult[0].obj->anim.localPosZ) -
                (u16)(obj)->anim.rotX;
            if (d > 0x8000)
            {
                d = (d - 0x10000) + 1;
            }
            if (d < -0x8000)
            {
                d = (d + 0x10000) - 1;
            }
            d = (s16)((u32)(u16)d >> 13);
            state[0x33a] = (u8)(entry[0].anim + gGroundBaddieAngleSectorOffsets[d]);
        }
        else if (gGroundBaddieTargetSearchResult[0].dist <= 70)
        {
            while ((entry[state[0x33a]].r & 1) != 0)
            {
                (((EnemyState*)state)->userData1)++;
                if (state[0x33a] > entry[0].anim)
                {
                    state[0x33a] = 1;
                }
            }
        }
    }
    if ((f32) * (u16*)(state + 0x2a4) < 0.8f * ((EnemyState*)state)->sightRange)
    {
        state[0x33a] = (u8)(entry[0].anim + 1);
    }
    while (entry[idx = state[0x33a]].mask != 0 &&
           (((EnemyState*)state)->controlFlags & entry[idx].mask) == 0)
    {
        (((EnemyState*)state)->userData1)++;
        if (state[0x33a] > entry[0].anim)
        {
            state[0x33a] = 1;
        }
    }
    *(u8*)(state + 0x2f2) = entry[state[0x33a]].r;
    *(u8*)(state + 0x2f3) = entry[state[0x33a]].g;
    *(u8*)(state + 0x2f4) = entry[state[0x33a]].b;
    baddieSetMove(obj, (int)state, entry[state[0x33a]].anim, entry[state[0x33a]].speed, 0, 3);
    ObjAnim_SetMoveProgress(&obj->anim,
                            *(f32*)(gBaddieMoveProgressTable + entry[state[0x33a]].anim * 4));
    (((EnemyState*)state)->userData1)++;
    if (state[0x33a] > entry[0].anim)
    {
        state[0x33a] = 1;
    }
}

void sharpClawUpdateAttack(GameObject* obj, u8* state)
{
    GameObject* player;
    u8* p20;
    u8* p28;
    u8 tableIdx;
    f32 tv;
    f32 fz;
    GroundBaddieSequenceTable* table;

    table = (GroundBaddieSequenceTable*)gBaddieFamilyTables;
    tableIdx = ((EnemyState*)state)->userData2;
    p20 = table[tableIdx].hitEntries;
    p28 = table[tableIdx].sequenceEntries;
    if (tableIdx == 5 && (((EnemyState*)state)->controlFlags & 0x800000) != 0)
    {
        mainSetBits(GAMEBIT_BaddieRelated1C8, 1);
    }
    if (((EnemyState*)state)->trackedObj != NULL &&
        ((GameObject*)((EnemyState*)state)->trackedObj)->anim.classId == 1)
    {
        requestKrazoaShrineMusic();
    }
    wispBaddiePlayMoveEventSfx(obj, state);
    tv = *(f32*)(state + 0x328);
    fz = 0.0f;
    if (tv != fz && *(u16*)(state + 0x338) != 0)
    {
        *(f32*)(state + 0x328) = tv - timeDelta;
        if (*(f32*)(state + 0x328) <= fz)
        {
            *(f32*)(state + 0x328) = fz;
            ((EnemyState*)state)->controlFlags |= (u64)BADDIE_CONTROL_SEQUENCE_DRIVEN;
            *(u16*)(state + 0x338) = (p28 + *(u16*)(state + 0x338) * 16)[10];
        }
    }
    if ((u8)wispBaddieProcessAnimEvent(obj, state, 1) == 0)
    {
        if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            player = Obj_GetPlayerObject();
            enemy_findNearbyEnemies(obj, 100.0f, 3, 16, gGroundBaddieTargetSearchResult);
            if (*(u16*)(state + 0x338) != 0)
            {
                {
                    u8* p28c = p28 + 12;
                    *(u8*)(state + 0x2f2) = (u8) * (u32*)(p28c + *(u16*)(state + 0x338) * 16);
                }
                baddieSetMove(obj, (int)state, (p28 + *(u16*)(state + 0x338) * 16)[8],
                            ((SeqEntry*)(p28 + *(u16*)(state + 0x338) * 16))->speed, 0,
                            (u8) * (u32*)(&p28[*(u16*)(state + 0x338) * 16 + 4]));
                ObjAnim_SetMoveProgress(
                    &obj->anim,
                    *(f32*)(gBaddieMoveProgressTable + (p28 + *(u16*)(state + 0x338) * 16)[8] * 4));
                *(u16*)(state + 0x338) = (p28 + *(u16*)(state + 0x338) * 16)[9];
            }
            else
            {
                if (player != NULL && ((((EnemyState*)state)->controlFlags & 0x800080) != 0 ||
                                       (void*)Player_GetTargetObject((int)player) == NULL))
                {
                    groundBaddiePickIdleMove(obj, state);
                }
                else
                {
                    groundBaddiePickNextMove(obj, state);
                }
            }
        }
        ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->hitVolumePriority = 0;
        ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->hitVolumeId = 0;
        if ((obj)->anim.currentMove == p20[8])
        {
            ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->hitVolumePriority = (s8) * (int*)(p20 + 4);
            ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->hitVolumeId = p20[9];
        }
        if ((obj)->anim.currentMove == p20[0x14])
        {
            ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->hitVolumePriority = (s8) * (int*)(p20 + 0x10);
            ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->hitVolumeId = p20[0x15];
        }
        if ((obj)->anim.currentMove == p20[0x20])
        {
            ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->hitVolumePriority = (s8) * (int*)(p20 + 0x1c);
            ((ObjHitsPriorityState*)(obj)->anim.hitReactState)->hitVolumeId = p20[0x21];
        }
        if ((state[0x323] & 8) == 0)
        {
            baddieTurnTowardPoint(obj, (int)state,
                        ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX,
                        ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ, 10, 0);
        }
    }
}

void sharpClawInit(int obj, u8* state)
{
    u8* setup = (u8*)((GameObject*)obj)->anim.placementData;
    f32 fz;
    f32 fz2;
    int z;

    ((EnemyState*)state)->flags2E4 = 11;
    ((EnemyState*)state)->flags2E4 |= 0x402B0LL;
    ((EnemyState*)state)->flags2E4 |= 0x3040;
    ((EnemyState*)state)->flags2E4 |= 0x40300000LL;
    ((EnemyState*)state)->flags2E4 |= 0xC00;
    ((EnemyState*)state)->animPlaySpeed = 0.005f;
    ((EnemyState*)state)->gravity = 0.17f;
    ((EnemyState*)state)->drag = 0.97f;
    ((EnemyState*)state)->moveId0 = 35;
    fz = 1.0f;
    ((EnemyState*)state)->moveSpeedScale0 = fz;
    ((EnemyState*)state)->moveId1 = 34;
    ((EnemyState*)state)->moveSpeedScale1 = 1.25f;
    ((EnemyState*)state)->moveId2 = 6;
    ((EnemyState*)state)->moveSpeedScale2 = fz;
    ((EnemyState*)state)->pathStep *= 10.0f;
    switch (((GameObject*)obj)->anim.romDefNo)
    {
    case 314:
        if (*(s8*)(setup + 0x27) != 0)
        {
            *(s16*)(state + 0x2b6) = 51;
        }
        ((EnemyState*)state)->sightRange = 110.0f;
        ((EnemyState*)state)->current = 40;
        ((EnemyState*)state)->userData2 = 0;
        break;
    case 17:
        if (*(s8*)(setup + 0x27) != 0)
        {
            *(s16*)(state + 0x2b6) = 51;
        }
        ((EnemyState*)state)->sightRange = 110.0f;
        ((EnemyState*)state)->current = 40;
        ((EnemyState*)state)->userData2 = 1;
        break;
    case 1505:
        if (*(s8*)(setup + 0x27) != 0)
        {
            *(s16*)(state + 0x2b6) = 1529;
        }
        ((EnemyState*)state)->sightRange = 110.0f;
        ((EnemyState*)state)->current = 50;
        ((EnemyState*)state)->userData2 = 2;
        break;
    case 1463:
        if (*(s8*)(setup + 0x27) != 0)
        {
            *(s16*)(state + 0x2b6) = 1530;
        }
        ((EnemyState*)state)->sightRange = 120.0f;
        ((EnemyState*)state)->current = 50;
        ((EnemyState*)state)->userData2 = 3;
        break;
    case 1464:
        if (*(s8*)(setup + 0x27) != 0)
        {
            *(s16*)(state + 0x2b6) = 1534;
        }
        ((EnemyState*)state)->sightRange = 110.0f;
        ((EnemyState*)state)->current = 60;
        ((EnemyState*)state)->userData2 = 4;
        break;
    case 1465:
        if (*(s8*)(setup + 0x27) != 0)
        {
            *(s16*)(state + 0x2b6) = 51;
        }
        ((EnemyState*)state)->sightRange = 110.0f;
        ((EnemyState*)state)->current = 1;
        ((EnemyState*)state)->userData2 = 1;
        break;
    case 1958:
        if (*(s8*)(setup + 0x27) != 0)
        {
            *(s16*)(state + 0x2b6) = 1957;
        }
        ((EnemyState*)state)->sightRange = 110.0f;
        ((EnemyState*)state)->current = 160;
        ((EnemyState*)state)->userData2 = 5;
        z = 0;
        ((EnemyState*)state)->moveId0 = z;
        fz2 = 1.0f;
        ((EnemyState*)state)->moveSpeedScale0 = fz2;
        ((EnemyState*)state)->moveId1 = 21;
        ((EnemyState*)state)->moveSpeedScale1 = 1.25f;
        ((EnemyState*)state)->moveId2 = z;
        ((EnemyState*)state)->moveSpeedScale2 = fz2;
        *(int*)(state + 0x36c) = (int)ObjModelChain_Alloc(&gGroundBaddieModelChainDesc, 1);
        ObjModelChain_SetOrigin((ObjModelChain*)*(int*)(state + 0x36c), 0.15f, 0.75f, -0.05f);
        *(int*)(obj + 0x108) = (int)baddieAfterUpdateBonesCb;
        ObjModelChain_SetEnabled((ObjModelChain*)*(int*)(state + 0x36c), 1);
        break;
    }
    if (*(s8*)(setup + 0x2e) != -1)
    {
        ((EnemyState*)state)->controlFlags |= 1;
    }
}

void groundBaddieHandlePaidTrigger(int obj, u8* state)
{
    GameObject* player;
    u8* setup;

    player = Obj_GetPlayerObject();
    setup = (u8*)((GameObject*)obj)->anim.placementData;
    if ((*gGameUIInterface)->isItemBeingUsed(446) != 0)
    {
        if (player != NULL && playerGetMoney(player) >= 25)
        {
            playerAddMoney(player, -25);
            mainSetBits(*(s16*)(setup + 0x1c), 1);
            *(u16*)(state + 0x338) = gGroundBaddieTriggerResponseSeq[2];
            ((GameObject*)obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
            setHudForceShowMask(2);
            (*gObjectTriggerInterface)->runSequence(2, (void*)obj, -1);
        }
        else
        {
            setHudForceShowMask(2);
            *(u16*)(state + 0x338) = gGroundBaddieTriggerResponseSeq[1];
            (*gObjectTriggerInterface)->runSequence(1, (void*)obj, -1);
        }
    }
    else
    {
        setHudForceShowMask(2);
        *(u16*)(state + 0x338) = gGroundBaddieTriggerResponseSeq[0];
        (*gObjectTriggerInterface)->runSequence(0, (void*)obj, -1);
    }
}

#define GROUND_BADDIE_PI 3.14159274f
#define GROUND_BADDIE_ANGLE_UNIT_SCALE 32768.0f
#define GROUND_BADDIE_PUSH_RADIUS 50.0f
#define GROUND_BADDIE_PUSH_MAX_DEPTH -20.0f

void groundBaddiePushPlayerOut(GameObject* obj, u8* state)
{
    GameObject* player;
    ObjPlacement* setup;
    f32 dy;
    f32 px0;
    f32 pz0;
    f32 sinA;
    f32 cosA;
    f32 base;
    f32 f5;
    f32 f2v;
    f32 dx;
    f32 dz;

    player = Obj_GetPlayerObject();
    setup = obj->anim.placement;
    dy = player->anim.localPosY - obj->anim.localPosY;
    dy = (dy >= 0.0f) ? dy : -dy;
    if (dy > GROUND_BADDIE_PUSH_RADIUS)
    {
        return;
    }
    px0 = setup->posX - GROUND_BADDIE_PUSH_RADIUS * mathSinf(GROUND_BADDIE_PI *
                                                             (f32)obj->anim.rotX /
                                                             GROUND_BADDIE_ANGLE_UNIT_SCALE);
    pz0 = setup->posZ - GROUND_BADDIE_PUSH_RADIUS * mathCosf(GROUND_BADDIE_PI *
                                                             (f32)obj->anim.rotX /
                                                             GROUND_BADDIE_ANGLE_UNIT_SCALE);
    dx = player->anim.worldPosX - px0;
    dz = player->anim.worldPosZ - pz0;
    if (sqrtf(dx * dx + dz * dz) < ((EnemyState*)state)->sightRange)
    {
        sinA = mathSinf(GROUND_BADDIE_PI * (f32)obj->anim.rotX / GROUND_BADDIE_ANGLE_UNIT_SCALE);
        cosA = mathCosf(GROUND_BADDIE_PI * (f32)obj->anim.rotX / GROUND_BADDIE_ANGLE_UNIT_SCALE);
        base = -(sinA * (px0 - sinA) + cosA * (pz0 - cosA));
        f5 = base + (sinA * player->anim.previousWorldPosX + cosA * player->anim.previousWorldPosZ);
        f2v = base + (sinA * player->anim.worldPosX + cosA * player->anim.worldPosZ);
        if (f2v > 0.0f)
        {
            if (!(f5 >= GROUND_BADDIE_PUSH_MAX_DEPTH))
            {
                return;
            }
            player->anim.worldPosX = player->anim.worldPosX - sinA * f5;
            player->anim.worldPosZ = player->anim.worldPosZ - cosA * f5;
            Obj_TransformWorldPointToLocal(player->anim.worldPosX, player->anim.worldPosY, player->anim.worldPosZ,
                                           &player->anim.localPosX, &player->anim.localPosY, &player->anim.localPosZ,
                                           player->anim.parent);
        }
    }
}

void guardClawUpdateWhileFrozen(GameObject* obj, u8* state, int wpad0, int wpad1, int wpad2, int wpad3, Vec* wpad4,
                                int wpad5)
{
    Sfx_PlayFromObject((int)obj, SFXTRIG_wp_pole1_c_23);
    ((EnemyState*)state)->flags2E8 |= 0x10;
}

int gGcRobotPatrolCurveInitData[2] = {2, 3};
f32 gGcRobotPatrolRiseAccel = 0.018f;
f32 gGcRobotPatrolCatchCooldown = 240.0f;

/* gcRobotPatrol (mikaladon_update): periodically dropped object; parented back to
 * the dropper via +0xC4 and announced with SFX 0x249. */
#define SEQOBJ11E_GCROBOT_DROP_OBJ 0x6b5

typedef void (*SeqObj11ESetMovePointerStateFn)(GameObject* obj, void* state, int moveId, f32 speed, int p5,
                                               int flags);

/* guardClaw_update: state-table driver: walks the 12-byte gSeq11EStateTable state
 * rows, advancing on GameBit + sequence flags and kicking the matching anim. */

typedef struct
{
    f32 animSpeed; /* 0x0 */
    u32 unk4;      /* 0x4 */
    u8 anim;       /* 0x8 */
    u8 next;       /* 0x9 */
    u8 alt;        /* 0xa */
    u8 flagB;      /* 0xb */
} Seq11ERow;

extern Seq11ERow gSeq11EStateTable[];

void guardClaw_update(GameObject* obj, u8* state)
{
    int* def = *(int**)&(obj)->anim.placementData;
    u32 flags;

    if (((EnemyState*)state)->userData1 == 2 && mainGetBit(*(s16*)((char*)def + 0x1c)) == 0)
    {
        (obj)->anim.resetHitboxFlags =
            (u8)((obj)->anim.resetHitboxFlags & ~INTERACT_FLAG_DISABLED);
        if ((obj)->anim.resetHitboxFlags & INTERACT_FLAG_ACTIVATED)
        {
            groundBaddieHandlePaidTrigger((int)obj, state);
        }
    }
    else
    {
        (obj)->anim.resetHitboxFlags =
            (u8)((obj)->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED);
    }
    flags = ((EnemyState*)state)->controlFlags;
    if (flags & BADDIE_CONTROL_JUST_TRIGGERED)
    {
        if (gSeq11EStateTable[((EnemyState*)state)->userData1].unk4 != 0)
        {
            ((EnemyState*)state)->controlFlags = flags | (u64)BADDIE_CONTROL_SEQUENCE_DRIVEN;
        }
    }
    flags = ((EnemyState*)state)->controlFlags;
    if (flags & BADDIE_CONTROL_SEQUENCE_DRIVEN)
    {
        int anim;
        u8* animTbl;

        if (((EnemyState*)state)->userData1 == 0)
        {
            if (flags & 0x20000000)
            {
                if (mainGetBit(*(s16*)((char*)def + 0x1c)) != 0)
                {
                    ((EnemyState*)state)->userData1 = gSeq11EStateTable[((EnemyState*)state)->userData1].alt;
                }
                else
                {
                    ((EnemyState*)state)->userData1 = gSeq11EStateTable[((EnemyState*)state)->userData1].next;
                }
            }
        }
        else if (((EnemyState*)state)->userData1 == 2)
        {
            if (mainGetBit(*(s16*)((char*)def + 0x1c)) != 0 || !(((EnemyState*)state)->controlFlags & 0x20000000))
            {
                ((EnemyState*)state)->userData1 = gSeq11EStateTable[((EnemyState*)state)->userData1].next;
            }
        }
        else if (((EnemyState*)state)->userData1 == 3)
        {
            if (mainGetBit(*(s16*)((char*)def + 0x1c)) != 0)
            {
                ((EnemyState*)state)->userData1 = gSeq11EStateTable[((EnemyState*)state)->userData1].alt;
            }
            else
            {
                ((EnemyState*)state)->userData1 = gSeq11EStateTable[((EnemyState*)state)->userData1].next;
            }
        }
        else
        {
            ((EnemyState*)state)->userData1 = gSeq11EStateTable[((EnemyState*)state)->userData1].next;
        }
        anim = (obj)->anim.currentMove;
        if (anim != (animTbl = (u8*)gSeq11EStateTable + 8)[((EnemyState*)state)->userData1 * 12])
        {
            if (animTbl[((EnemyState*)state)->userData1 * 12] != 0 &&
                animTbl[((EnemyState*)state)->userData1 * 12] != 4)
            {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_baddie_eggsnatch_carry3);
            }
            ((SeqObj11ESetMovePointerStateFn)baddieSetMove)(
                obj, state, animTbl[((EnemyState*)state)->userData1 * 12],
                *(f32*)((u8*)gSeq11EStateTable + ((EnemyState*)state)->userData1 * 12), 0, 0xf);
        }
    }
    if (gSeq11EStateTable[((EnemyState*)state)->userData1].flagB != 0)
    {
        groundBaddiePushPlayerOut(obj, state);
    }
}

void guardClaw_init(GameObject* obj, u8* state)
{
    int* sub = *(int**)&(obj)->anim.placementData;
    f32 fz;
    ((EnemyState*)state)->sightRange = 200.0f;
    ((EnemyState*)state)->aggroRange = 300.0f;
    ((EnemyState*)state)->flags2E4 = 1;
    ((EnemyState*)state)->flags2E4 |= 0xC80;
    ((EnemyState*)state)->animPlaySpeed = 0.0055555557f;
    ((EnemyState*)state)->gravity = 0.17f;
    ((EnemyState*)state)->drag = 0.97f;
    ((EnemyState*)state)->moveId0 = 0;
    fz = 1.0f;
    ((EnemyState*)state)->moveSpeedScale0 = fz;
    ((EnemyState*)state)->moveId1 = 0;
    ((EnemyState*)state)->moveSpeedScale1 = fz;
    ((EnemyState*)state)->moveId2 = 0;
    ((EnemyState*)state)->moveSpeedScale2 = fz;
    if (*((s8*)sub + 0x2e) != -1)
    {
        *(int*)&((EnemyState*)state)->controlFlags |= 1;
    }
    (obj)->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
}

GameObject* gcRobotLight_init(GameObject* obj, int childId)
{
    ObjPlacement* sub;
    u8* setup;

    sub = (ObjPlacement*)(obj->anim.placementDataAddress);
    Obj_GetPlayerObject();
    if (Obj_IsLoadingLocked() == 0)
        return NULL;
    setup = (u8*)Obj_AllocObjectSetup(36, childId);
    *(s16*)(setup + 0) = childId;
    ((ObjPlacement*)setup)->color[0] = sub->color[0];
    ((ObjPlacement*)setup)->color[2] = sub->color[2];
    ((ObjPlacement*)setup)->color[1] = 1;
    ((ObjPlacement*)setup)->color[3] = sub->color[3];
    ((ObjPlacement*)setup)->posX = obj->anim.localPosX;
    ((ObjPlacement*)setup)->posY = obj->anim.localPosY;
    ((ObjPlacement*)setup)->posZ = obj->anim.localPosZ;
    ((Seq11EChildSetup*)setup)->unk19 = 0;
    ((Seq11EChildSetup*)setup)->unk20 = 149;
    return Obj_SetupObject((ObjPlacement*)setup, 5, obj->anim.mapEventSlot, -1, obj->anim.parent);
}

Seq11ERow gSeq11EStateTable[6] = {
    {3.0f, 0x1, 0, 1, 4, 1}, {2.0f, 0x0, 1, 2, 2, 1}, {3.0f, 0x1, 2, 3, 3, 1},
    {2.0f, 0x0, 7, 0, 4, 1}, {2.0f, 0x0, 3, 5, 5, 0}, {3.5f, 0x1, 4, 5, 5, 0},
};

typedef void (*SeqObj11ESetMovePointerStateFn)(GameObject* obj, void* state, int moveId, f32 speed, int p5,
                                               int flags);

extern int gGcRobotPatrolCurveInitData[2];
extern f32 gGcRobotPatrolRiseAccel;
extern f32 gGcRobotPatrolCatchCooldown;

void gcRobotPatrol_updateWhileFrozen(int obj, u8* state, int unused, int msg, int wpad0, int wpad1, Vec* wpad2, int wpad3)
{
    int sub;
    f32 fz;

    sub = ((GameObject*)obj)->anim.placementDataAddress;
    if (msg == 16 || msg == 17)
    {
        return;
    }
    Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_pole1_c_23);
    Sfx_PlayFromObject((u32)obj, SFXTRIG_en_lrope_powerdown);
    ((EnemyState*)state)->flags2E8 |= 0x8;
    *(f32*)(state + 0x32c) = (f32)(u32)(u16) * (s16*)(sub + 0x2c);
    baddieSetMove((GameObject*)obj, (int)state, 1, 2.5f, 0, 0);
    ((EnemyState*)state)->flags2E4 &= ~0x20LL;
    fz = 0.0f;
    ((GameObject*)obj)->anim.velocityZ = 0.0f;
    ((GameObject*)obj)->anim.velocityY = fz;
    ((GameObject*)obj)->anim.velocityX = fz;
}

/* gcRobotPatrol_update: main update: child-zap timer, curve follow, heading steps,
 * landing sfx, light-pulse fx, child spark spawn. */

typedef struct
{
    u8 pad[8];
    f32 a;
    f32 b;
    f32 c;
    f32 d;
} SeqFxParams;

void gcRobotPatrol_update(GameObject* obj, u8* state)
{
    int* def;
    RomCurveWalker* path;
    int attached;
    s16 spd;
    SeqFxParams fx;

    def = *(int**)&obj->anim.placementData;
    path = *(RomCurveWalker**)state;
    if (*(f32*)(state + 0x32c) > 0.0f)
    {
        GameObject* child = obj->childObjs[0];
        if (child != 0)
        {
            Obj_FreeObject(child);
            ObjLink_DetachChild(obj, obj->childObjs[0]);
            obj->childObjs[0] = 0;
        }
        *(f32*)(state + 0x32c) = *(f32*)(state + 0x32c) - timeDelta;
        if (*(f32*)(state + 0x32c) <= 0.0f)
        {
            *(f32*)(state + 0x32c) = 0.0f;
            ((EnemyState*)state)->flags2E4 |= 0x20;
            Sfx_StopObjectChannel((u32)obj, 4);
            ((SeqObj11ESetMovePointerStateFn)baddieSetMove)(obj, state, 0, 1.0f, 0, 0);
        }
        else if (!(((EnemyState*)state)->flags2E4 & 0x20))
        {
            return;
        }
    }
    if (((EnemyState*)state)->controlFlags & BADDIE_CONTROL_PATH_FOLLOW)
    {
        int step;

        if (Curve_AdvanceAlongPath(&path->curve, ((EnemyState*)state)->pathStep) != 0 || path->atSegmentEnd != 0)
        {
            if ((*gRomCurveInterface)->goNextPoint(path) != 0)
            {
                if ((*gRomCurveInterface)
                        ->initCurve(*(RomCurveWalker**)state, obj, 700.0f, (int*)&gGcRobotPatrolCurveInitData, -1) != 0)
                {
                    ((EnemyState*)state)->controlFlags &= ~(u64)BADDIE_CONTROL_PATH_FOLLOW;
                }
            }
        }
        obj->anim.velocityX = (path->posX - obj->anim.localPosX) / timeDelta;
        obj->anim.velocityZ = (path->posZ - obj->anim.localPosZ) / timeDelta;
        step = (s8) * ((u8*)def + 0x2a);
        if (step == 0)
        {
            baddieTurnTowardPoint(obj, (int)state, path->posX, path->posZ, 0xf, 0);
        }
        else if (((EnemyState*)state)->controlFlags & BADDIE_CONTROL_PATH_FOLLOW)
        {
            spd = step << 8;
            if ((int)(10.0f * path->tangentY) >= 0)
            {
                step = spd;
            }
            else
            {
                step = -spd;
            }
            obj->anim.rotX = obj->anim.rotX - step;
            baddieTurnTowardPoint(obj, (int)state, path->posX, path->posZ, 0xf, 0);
            if ((int)(10.0f * path->tangentY) >= 0)
            {
                step = spd;
            }
            else
            {
                step = -spd;
            }
            obj->anim.rotX += step;
        }
        else
        {
            step = ((int)(10.0f * path->tangentY) >= 0) ? step : -step;
            obj->anim.rotX += step;
        }
        if (obj->anim.localPosY - path->posY < -1.0f)
        {
            if (Sfx_IsPlayingFromObject((u32)obj, SFXTRIG_dn_boar1_c_18d) == 0)
            {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_18d);
            }
            ((EnemyState*)state)->userData1 = 1;
        }
        else
        {
            ((EnemyState*)state)->userData1 = 0;
        }
    }
    else
    {
        if (obj->anim.localPosY - ((ObjPlacement*)def)->posY < -0.4f)
        {
            if (Sfx_IsPlayingFromObject((u32)obj, SFXTRIG_dn_boar1_c_18d) == 0)
            {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_18d);
            }
            ((EnemyState*)state)->userData1 = 1;
        }
        else
        {
            ((EnemyState*)state)->userData1 = 0;
        }
        obj->anim.rotX += *(s8*)((char*)def + 0x2a);
    }
    if (((EnemyState*)state)->userData1 != 0)
    {
        obj->anim.velocityY += gGcRobotPatrolRiseAccel * timeDelta;
    }
    if (obj->objectFlags & OBJECT_OBJFLAG_RENDERED)
    {
        f32 z = 0.0f;
        fx.b = z;
        fx.c = z;
        fx.d = z;
        fx.a = 1.0f;
        objfx_spawnLightPulse(obj, 0.5f, 2, 0, 6, 0.25f, &fx);
        fx.c = 12.0f;
        objfx_spawnMaskedHitEffect(obj, 0.4f, 1, 6, 0x20, &fx);
        fx.b = 0.0f;
        z = -30.0f;
        fx.c = z;
        fx.d = z;
    }
    if (obj->anim.velocityY < -0.5f)
    {
        obj->anim.velocityY = -0.5f;
    }
    else if (obj->anim.velocityY > 0.5f)
    {
        obj->anim.velocityY = 0.5f;
    }
    if (0.0f == *(f32*)(state + 0x32c))
    {
        GameObject* child2;

        if (*(s8*)((char*)def + 0x2e) != -1 && (child2 = obj->childObjs[0]) != 0 &&
            gcRobotLightBeam_isPlayerCaught(child2) != 0)
        {
            ObjHits_RecordObjectHit(Obj_GetPlayerObject(), obj, 0x16, 2, 0);
            gcRobotLight_init(obj, 0x3b2);
            Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_rolovr_6);
            *(f32*)(state + 0x32c) = gGcRobotPatrolCatchCooldown;
        }
        if ((int)randomGetRange(0, (int)(1000.0f * oneOverTimeDelta)) == 0)
        {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_sp_literun114);
        }
        child2 = obj->childObjs[0];
        if (child2 != 0)
        {
            ObjTextureRuntimeSlot* tex = objFindTexture(child2, 0, 0);
            int v;
            if (tex != 0)
            {
                v = tex->offsetS - 0x3c;
                if (v < 0)
                {
                    v += 0x2710;
                }
                tex->offsetS = v;
            }
        }
        else
        {
            GameObject* newObj;
            int flag;

            if (*(s8*)((char*)def + 0x2a) != 0)
            {
                attached = 1;
            }
            else
            {
                attached = 0;
            }
            newObj = gcRobotLight_init(obj, 0x639);
            flag = 0;
            if (*(s8*)((char*)def + 0x2a) != 0 && !(((EnemyState*)state)->controlFlags & BADDIE_CONTROL_PATH_FOLLOW))
            {
                flag = 1;
            }
            newObj->userData1 = flag;
            ObjLink_AttachChild(obj, newObj, attached);
        }
    }
}

void gcRobotPatrol_init(GameObject* obj, int state)
{
    f32 fz;

    ((EnemyState*)state)->sightRange = 60.0f;
    ((EnemyState*)state)->flags2E4 = 41;
    ((EnemyState*)state)->flags2E4 |= 0x7000;
    ((EnemyState*)state)->flags2E4 |= 0x20000LL;
    ((EnemyState*)state)->animPlaySpeed = 0.005f;
    ((EnemyState*)state)->gravity = 0.006f;
    ((EnemyState*)state)->drag = 0.99f;
    ((EnemyState*)state)->moveId0 = 0;
    fz = 1.0f;
    ((EnemyState*)state)->moveSpeedScale0 = fz;
    ((EnemyState*)state)->moveId1 = 0;
    ((EnemyState*)state)->moveSpeedScale1 = fz;
    ((EnemyState*)state)->moveId2 = 0;
    ((EnemyState*)state)->moveSpeedScale2 = fz;
    *(f32*)(state + 0x32c) = 0.0f;
    obj->anim.hitboxScale = 100.0f;
    Sfx_AddLoopedObjectSound((u32)obj, SFXTRIG_tr_bcrek1_c);
}

const f32 gGcRobotPatrolZero = 0.0f;
const f32 gMikaladonZero = 0.0f;
const f32 gMikaladonDefaultPeriod = 60.0f;

void mikaladon_updateWhileFrozen(int obj, u8* state, int unused, int msg, int wpad0, int wpad1, Vec* wpad2, int wpad3)
{
    if (msg == 16 || msg == 17)
    {
        return;
    }
    Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_248);
    ((EnemyState*)state)->current = 0;
    ((EnemyState*)state)->flags2E4 |= 0x20;
    ((EnemyState*)state)->flags2E8 |= 0x8;
}

#define MAGICPLANT_OBJFLAG_PARENT_SLACK 0x1000

/* Spit projectile spawned by kooshy_spawnProjectile; retail OBJECTS.bin name
   "KaldachomSp" (DLL 0xD7 kaldachompspit), shared with the snowworm spitter. */
#define KALDACHOM_SPIT_OBJ 0x51b

/* The magic-plant's one particle-fx effect (spawned per hit-count in the
   attack handler). */
#define MAGICPLANT_PARTFX          0x802
#define MAGICPLANT_HIT_VOLUME_SLOT 0xe

enum MikaladonVerticalPhase
{
    MIKALADON_PHASE_ORBIT = 0,
    MIKALADON_PHASE_DESCEND = 1,
    MIKALADON_PHASE_ASCEND = 2
};

#define MIKALADON_ORBIT_ANGLE_SPEED     75.0f
#define MIKALADON_TRIGGER_RADIUS_SCALE  1.3f
#define MIKALADON_DESCENT_SPEED         0.5f
#define MIKALADON_DESCENT_DISTANCE      500.0f
#define MIKALADON_ASCENT_SPEED          1.5f
#define MIKALADON_DROP_INTERVAL         100
#define MIKALADON_DROP_HEIGHT_OFFSET    5.0f
#define MIKALADON_AMBIENT_SFX_MIN_DELAY 60
#define MIKALADON_AMBIENT_SFX_MAX_DELAY 120

static inline f32 mikaladon_randomAmbientSfxDelay(void)
{
    return (f32)(int)randomGetRange(MIKALADON_AMBIENT_SFX_MIN_DELAY, MIKALADON_AMBIENT_SFX_MAX_DELAY);
}

static inline void mikaladon_spawnDrop(GameObject* obj)
{
    MikaladonDropSetup* setup;
    GameObject* spawned;

    setup = (MikaladonDropSetup*)Obj_AllocObjectSetup(sizeof(MikaladonDropSetup), SEQOBJ11E_GCROBOT_DROP_OBJ);
    setup->base.posX = obj->anim.localPosX;
    setup->base.posY = MIKALADON_DROP_HEIGHT_OFFSET + obj->anim.localPosY;
    setup->base.posZ = obj->anim.localPosZ;
    setup->base.color[0] = 1;
    setup->base.color[1] = 1;
    setup->base.color[2] = 0xff;
    setup->base.color[3] = 0xff;
    spawned = loadObjectAtObject(obj, &setup->base);
    if (spawned != NULL)
    {
        spawned->ownerObj = obj;
        Sfx_PlayFromObject((u32)obj, SFXTRIG_id_249);
    }
}

/* mikaladon_update: firefly hover update: circle drift, bob between heights,
 * periodically drop a spawned object, ambient sfx timers. */
void mikaladon_update(GameObject* obj, MikaladonState* state)
{
    f32 y;
    f32 sinOut;
    f32 cosOut;

    state->actor.orbitAngle =
        MIKALADON_ORBIT_ANGLE_SPEED * timeDelta + (f32)(u32)state->actor.orbitAngle;
    angleToVec2Precise(state->actor.orbitAngle, &sinOut, &cosOut);
    sinOut = sinOut * ((EnemyState*)state)->aggroRange + state->actor.orbitCenterX;
    cosOut = cosOut * ((EnemyState*)state)->aggroRange + state->actor.orbitCenterZ;
    if (state->actor.verticalPhase == MIKALADON_PHASE_ORBIT)
    {
        f32 dx;
        f32 dz;

        y = obj->anim.localPosY;
        dx = state->actor.orbitCenterX -
             ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX;
        dz = state->actor.orbitCenterZ -
             ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ;
        if (sqrtf(dx * dx + dz * dz) <= MIKALADON_TRIGGER_RADIUS_SCALE * ((EnemyState*)state)->aggroRange)
        {
            state->actor.verticalPhase = MIKALADON_PHASE_DESCEND;
            state->actor.dropTimer = 0;
        }
    }
    else if (state->actor.verticalPhase == MIKALADON_PHASE_DESCEND)
    {
        y = obj->anim.localPosY - MIKALADON_DESCENT_SPEED * timeDelta;
        if (y <= state->actor.homeY - MIKALADON_DESCENT_DISTANCE)
        {
            state->actor.verticalPhase = MIKALADON_PHASE_ASCEND;
        }
        else
        {
            state->actor.dropTimer =
                (f32)(u32)state->actor.dropTimer + timeDelta;
            if (state->actor.dropTimer > MIKALADON_DROP_INTERVAL)
            {
                state->actor.dropTimer = 0;
                if (Obj_IsLoadingLocked() != 0)
                {
                    MikaladonDropSetup* setup;
                    GameObject* spawned;

                    setup = (MikaladonDropSetup*)Obj_AllocObjectSetup(sizeof(MikaladonDropSetup),
                                                                     SEQOBJ11E_GCROBOT_DROP_OBJ);
                    setup->base.posX = obj->anim.localPosX;
                    setup->base.posY = MIKALADON_DROP_HEIGHT_OFFSET + obj->anim.localPosY;
                    setup->base.posZ = obj->anim.localPosZ;
                    setup->base.color[0] = 1;
                    setup->base.color[1] = 1;
                    setup->base.color[2] = 0xff;
                    setup->base.color[3] = 0xff;
                    spawned = loadObjectAtObject(obj, &setup->base);
                    if (spawned != NULL)
                    {
                        spawned->ownerObj = obj;
                        Sfx_PlayFromObject((u32)obj, SFXTRIG_id_249);
                    }
                }
            }
        }
    }
    else
    {
        y = MIKALADON_ASCENT_SPEED * timeDelta + obj->anim.localPosY;
        if (y >= state->actor.homeY)
        {
            state->actor.verticalPhase = MIKALADON_PHASE_ORBIT;
        }
    }
    obj->anim.velocityX = oneOverTimeDelta * (sinOut - obj->anim.localPosX);
    obj->anim.velocityY = oneOverTimeDelta * (y - obj->anim.localPosY);
    obj->anim.velocityZ = oneOverTimeDelta * (cosOut - obj->anim.localPosZ);
    baddieTurnTowardLookDir(obj, state, 0xf, 7.5f, 1.0f, 0);
    state->actor.ambientSfxTimer -= timeDelta;
    if (state->actor.ambientSfxTimer <= gMikaladonZero)
    {
        state->actor.ambientSfxTimer =
            (f32)(int)randomGetRange(MIKALADON_AMBIENT_SFX_MIN_DELAY, MIKALADON_AMBIENT_SFX_MAX_DELAY);
        Sfx_PlayFromObject((u32)obj, SFXTRIG_id_31);
    }
    state->actor.loopSfxTimer -= timeDelta;
    if (state->actor.loopSfxTimer <= gMikaladonZero)
    {
        state->actor.loopSfxTimer = gMikaladonDefaultPeriod;
        Sfx_PlayFromObject((u32)obj, SFXTRIG_id_24a);
    }
}

void mikaladon_init(GameObject* obj, MikaladonState* state)
{
    f32 zero;
    f32 lblA;
    f32 a, b;

    zero = gMikaladonDefaultPeriod;
    ((EnemyState*)state)->sightRange = zero;
    ((EnemyState*)state)->flags2E4 = 1;
    ((EnemyState*)state)->animPlaySpeed = 0.01f;
    ((EnemyState*)state)->gravity = 0.006f;
    lblA = 1.0f;
    ((EnemyState*)state)->drag = lblA;
    ((EnemyState*)state)->moveId0 = 1;
    ((EnemyState*)state)->moveSpeedScale0 = lblA;
    ((EnemyState*)state)->moveId1 = 3;
    ((EnemyState*)state)->moveSpeedScale1 = lblA;
    ((EnemyState*)state)->moveId2 = 1;
    ((EnemyState*)state)->moveSpeedScale2 = lblA;
    state->actor.orbitCenterX = obj->anim.localPosX;
    state->actor.homeY = obj->anim.localPosY;
    state->actor.orbitCenterZ = obj->anim.localPosZ;
    state->actor.verticalPhase = MIKALADON_PHASE_ORBIT;
    state->actor.dropTimer = 0;
    state->actor.orbitAngle = 0;
    state->actor.loopSfxTimer = zero;
    state->actor.ambientSfxTimer = zero;
    ((EnemyState*)state)->pathStep = 8.0f;

    angleToVec2Precise(state->actor.orbitAngle, &a, &b);
    obj->anim.localPosX =
        a * ((EnemyState*)state)->aggroRange + state->actor.orbitCenterX;
    obj->anim.localPosZ =
        b * ((EnemyState*)state)->aggroRange + state->actor.orbitCenterZ;
}

int gVambatCurveInitData[2] = {2, 3};
u8 gMagicPlantSeqEntryTable[8] = {1, 1, 3, 2, 0, 0, 0, 0};
int gWeevilCurveInitData[2] = {2, 3};

/* The Firebat variant of the shared Vambat/Firebat family (retail
   OBJECTS.bin name "Firebat", DLL 0xC9); it alone runs with userData2 set. */
#define MAGICPLANT_FIREBAT_SEQID 0x7c6

void vambat_updateWhileFrozen(int obj, u8* state, int unused, int msgFlag, int wpad0, int wpad1, Vec* wpad2, int wpad3)
{
    EnemyState* bs = (EnemyState*)state;

    if (bs->userData2 != 0)
    {
        if (msgFlag == 16)
        {
            bs->flags2E8 = bs->flags2E8 | 0x28;
            Sfx_PlayFromObject(obj, SFXTRIG_baddie_mika_wingflap);
            bs->current = 0;
        }
    }
    else if (msgFlag != 17)
    {
        if (msgFlag == 16)
        {
            bs->flags2E8 = bs->flags2E8 | 0x20;
        }
        else
        {
            bs->flags2E8 = bs->flags2E8 | 0x8;
            Sfx_PlayFromObject(obj, SFXTRIG_baddie_mika_wingflap);
            bs->current = 0;
        }
    }
}

static void vambat_resetAmbientSfxTimer(u8* state)
{
    if (*(f32*)(state + 0x328) <= 0.0f)
    {
        *(f32*)(state + 0x328) = 60.0f;
    }
}

void vambat_updateIdle(GameObject* obj, int state)
{
    ObjHitsPriorityState* hitState;
    RomCurveWalker* curve;
    f32 vec[3];
    EnemyState* bs = (EnemyState*)state;

    curve = *(RomCurveWalker**)state;
    if ((obj)->anim.hitReactState != NULL)
    {
        hitState = (ObjHitsPriorityState*)(obj)->anim.hitReactState;
        hitState->suppressOutgoingHits = 0;
    }
    if (bs->userData2 != 0)
    {
        bs->flags2E8 = bs->flags2E8 | 0x80;
    }
    if ((bs->controlFlags & BADDIE_CONTROL_PATH_FOLLOW) != 0)
    {
        if (Curve_AdvanceAlongPath(&curve->curve, bs->pathStep) != 0 ||
            curve->atSegmentEnd != 0)
        {
            if ((*gRomCurveInterface)->goNextPoint(curve) != 0)
            {
                if ((*gRomCurveInterface)
                        ->initCurve(*(RomCurveWalker**)state, (void*)obj, 7e+02f, gVambatCurveInitData, -1) != 0)
                {
                    bs->controlFlags = bs->controlFlags & ~(u64)BADDIE_CONTROL_PATH_FOLLOW;
                }
            }
        }

        baddieTurnTowardPoint(obj, state, curve->posX, curve->posZ, 0xf, 0);

        vec[0] = curve->posX - (obj)->anim.localPosX;
        vec[1] = curve->posY - (obj)->anim.localPosY;
        vec[2] = curve->posZ - (obj)->anim.localPosZ;
        enemy_steerVelocityToward(obj, (void*)state, vec, 1.5f, 0.75f, 0.15f, 1);

        *(f32*)(state + 0x324) = *(f32*)(state + 0x324) + timeDelta;
        if (*(f32*)(state + 0x324) > 3.6e+02f)
        {
            bs->flags2E4 = bs->flags2E4 & ~(u64)0x10000;
            *(f32*)(state + 0x324) = 0.0f;
        }
    }

    baddieTurnTowardLookDir(obj, (void*)state, 0xf, 1e+01f, 1.0f, 0);

    *(f32*)(state + 0x328) = *(f32*)(state + 0x328) - timeDelta;
    if (*(f32*)(state + 0x328) <= 0.0f)
    {
        *(f32*)(state + 0x328) = 60.0f;
        Sfx_PlayFromObject((int)obj, SFXTRIG_mn_heart1_c);
    }
    *(f32*)(state + 0x32c) = 0.0f;
}

void vambat_updateEngaged(GameObject* obj, int state)
{
    RomCurveWalker* curve;
    f32 vec[3];
    f32 worldPos[3];
    int gridB[2];
    int gridA[2];
    u8 hitOut;
    GameObject* trackedObj;
    EnemyState* bs = (EnemyState*)state;

    curve = *(RomCurveWalker**)state;
    if (bs->userData2 != 0)
    {
        bs->flags2E8 = bs->flags2E8 | 0x80;
    }
    if ((bs->controlFlags & BADDIE_CONTROL_JUST_TRIGGERED) != 0)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_mika_bombwhistle);
    }
    if ((bs->controlFlags & BADDIE_CONTROL_PATH_FOLLOW) != 0)
    {
        if (Curve_AdvanceAlongPath(&curve->curve, 2.0f * bs->pathStep) != 0 ||
            curve->atSegmentEnd != 0)
        {
            if ((*gRomCurveInterface)->goNextPoint(curve) != 0)
            {
                if ((*gRomCurveInterface)
                        ->initCurve(*(RomCurveWalker**)state, (void*)obj, 7e+02f, gVambatCurveInitData, -1) != 0)
                {
                    bs->controlFlags = bs->controlFlags & ~(u64)BADDIE_CONTROL_PATH_FOLLOW;
                }
            }
        }
    }
    ObjHits_SetHitVolumeSlot(&obj->anim, MAGICPLANT_HIT_VOLUME_SLOT, 1, 0);
    trackedObj = bs->trackedObj;
    vec[0] = trackedObj->anim.localPosX - (obj)->anim.localPosX;
    vec[1] = (25.0f + trackedObj->anim.localPosY) - (obj)->anim.localPosY;
    vec[2] = trackedObj->anim.localPosZ - (obj)->anim.localPosZ;
    PSVECMag((Vec*)vec);
    *(f32*)(state + 0x32c) = *(f32*)(state + 0x32c) + timeDelta;
    if (*(u32*)(state + 0x340) != 0 || *(f32*)(state + 0x32c) > 3.6e+02f)
    {
        bs->flags2E4 = bs->flags2E4 | 0x10000LL;
        *(f32*)(state + 0x324) = 0.0f;
        *(f32*)(state + 0x32c) = 0.0f;
    }
    else
    {
        worldPos[0] = (obj)->anim.localPosX;
        worldPos[1] = (obj)->anim.localPosY;
        worldPos[2] = (obj)->anim.localPosZ;
        voxmaps_worldToGrid(worldPos, (s16*)gridA);
        worldPos[0] = curve->posX;
        worldPos[1] = curve->posY;
        worldPos[2] = curve->posZ;
        voxmaps_worldToGrid(worldPos, (s16*)gridB);
        /* BUG: precedence - `!` binds before `&`, so this is (controlFlags == 0) & 0x01000000,
         * which is always false; the line-of-sight abort below can never fire. The author
         * almost certainly meant !(controlFlags & 0x01000000). */
        if (!bs->controlFlags & 0x01000000)
        {
            if (voxmaps_traceLine((VoxPos*)gridB, (VoxPos*)gridA, NULL, &hitOut, 0) == 0)
            {
                bs->flags2E4 = bs->flags2E4 | 0x10000LL;
                *(f32*)(state + 0x324) = 0.0f;
                *(f32*)(state + 0x32c) = 0.0f;
            }
        }
    }
    enemy_steerVelocityToward(obj, (void*)state, vec, 1.5f, 0.75f, 0.15f, 1);
    baddieTurnTowardLookDir(obj, (void*)state, 0xf, 1e+01f, 1.0f, 0);
}

void vambat_init(GameObject* obj, int state)
{
    f32 pathStepInit;
    f32 initSpeed;
    EnemyState* bs = (EnemyState*)state;

    bs->sightRange = 4e+01f;
    bs->flags2E4 = 0x1009;
    bs->animPlaySpeed = 0.02f;
    bs->gravity = 0.1f;
    bs->drag = 0.97f;
    bs->moveId0 = 0;
    initSpeed = 1.5f;
    bs->moveSpeedScale0 = initSpeed;
    bs->moveId1 = 1;
    pathStepInit = 1.0f;
    bs->moveSpeedScale1 = pathStepInit;
    bs->moveId2 = 0;
    bs->moveSpeedScale2 = initSpeed;
    *(f32*)(state + 0x324) = 0.0f;
    *(f32*)(state + 0x328) = 0.0f;
    *(f32*)(state + 0x32c) = 0.0f;
    bs->pathStep = pathStepInit;
    switch (obj->anim.romDefNo)
    {
    case MAGICPLANT_FIREBAT_SEQID:
        bs->userData2 = 1;
        break;
    default:
        bs->userData2 = 0;
        break;
    }
}

void magicplantSpawnMovePuffs(GameObject* obj, int state)
{
    u8 count = 0;
    EnemyState* bs = (EnemyState*)state;
    switch (obj->anim.currentMove)
    {
    case 1:
        count = 1;
        break;
    case 2:
        count = 1;
        break;
    case 3:
        count = 1;
        break;
    case 5:
        if ((bs->controlFlags & BADDIE_CONTROL_JUST_TRIGGERED) != 0)
        {
            count = 0xa;
        }
        break;
    case 7:
        break;
    }
    if (count != 0 && (bs->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) == 0)
    {
        u8 spawn = count;
        while (spawn != 0)
        {
            (*gPartfxInterface)->spawnObject((void*)obj, MAGICPLANT_PARTFX, NULL, 2, -1, NULL);
            spawn--;
        }
    }
}

void kooshy_spawnProjectile(GameObject* obj, int state)
{
    ObjPlacement* fx;
    int newObj;

    if ((u8)Obj_IsLoadingLocked() != 0)
    {
        fx = (ObjPlacement*)Obj_AllocObjectSetup(0x24, KALDACHOM_SPIT_OBJ);
        fx->posX = (obj)->anim.localPosX;
        fx->posY = 14.0f + (obj)->anim.localPosY;
        fx->posZ = (obj)->anim.localPosZ;
        fx->color[0] = 1;
        fx->color[1] = 1;
        fx->color[2] = 0xff;
        fx->color[3] = 0xff;
        newObj = (int)Obj_SetupObject(fx, 5, -1, -1, 0);
        if ((void*)newObj != NULL)
        {
            ((GameObject*)newObj)->anim.velocityX =
                0.02f * (((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX - fx->posX);
            {
                ((GameObject*)newObj)->anim.velocityY =
                    0.02f * ((14.0f + ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosY +
                              (f32)(s32)randomGetRange(-10, 10)) -
                             fx->posY);
                ((GameObject*)newObj)->anim.velocityZ =
                    0.02f * (((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ - fx->posZ);
            }
            *(int*)&((GameObject*)newObj)->ownerObj = (int)obj;
        }
        Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_blooplaugh2);
    }
}

void kooshy_updateWhileFrozen(GameObject* obj, u8* state, int attacker, int msgFlag, int hitId, int damage, Vec* wpad0,
                              int wpad1)
{
    if ((obj)->anim.currentMove == 1)
    {
        if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            return;
        }
    }
    if (msgFlag == 0x10)
    {
        ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x20;
    }
    else
    {
        ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x8;
        if (damage > (s32)((EnemyState*)state)->current)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_sc_walkstep);
            ((EnemyState*)state)->current = 0;
        }
        else
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_sc_runstep);
            ((EnemyState*)state)->current = (u16)(((EnemyState*)state)->current - damage);
        }
    }
}
void kooshy_updateIdle(GameObject* obj, int state)
{
    u32 hit;
    u8 losDetected;
    f32 worldPos[3];
    f32 vec[3];
    int gridB[2];
    int gridA[2];
    u8 hitOut;
    u8 flagByte;
    u32 rnd;
    s16 angle;

    ((EnemyState*)state)->userData2 = ((EnemyState*)state)->userData2 & 0x7f;
    losDetected = 0;
    vec[0] = (obj)->anim.localPosX - ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX;
    vec[1] = (obj)->anim.localPosY - ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosY;
    vec[2] = (obj)->anim.localPosZ - ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ;
    if (PSVECMag((Vec*)vec) < 400.0f &&
        (((GameObject*)((EnemyState*)state)->trackedObj)->objectFlags & MAGICPLANT_OBJFLAG_PARENT_SLACK) == 0)
    {
        worldPos[0] = (obj)->anim.localPosX;
        worldPos[1] = 10.0f + (obj)->anim.localPosY;
        worldPos[2] = (obj)->anim.localPosZ;
        voxmaps_worldToGrid(worldPos, (s16*)gridA);
        {
            GameObject* trackedObj = ((EnemyState*)state)->trackedObj;
            worldPos[0] = trackedObj->anim.localPosX;
            worldPos[1] = 20.0f + trackedObj->anim.localPosY;
            worldPos[2] = trackedObj->anim.localPosZ;
        }
        voxmaps_worldToGrid(worldPos, (s16*)gridB);
        hit = voxmaps_traceLine((VoxPos*)gridB, (VoxPos*)gridA, NULL, &hitOut, 0) & 0xff;
        if (hit != 0)
        {
            GameObject* trackedObj = ((EnemyState*)state)->trackedObj;
            baddieTurnTowardPoint(obj, state, trackedObj->anim.localPosX,
                        trackedObj->anim.localPosZ, 0x14, 0);
            angle = (s16)(getAngle(vec[0], vec[2]) - (u16)(obj)->anim.rotX);
            if (angle > 0x8000)
                angle = (angle - 0x10000) + 1;
            if (angle < -0x8000)
                angle = (angle + 0x10000) - 1;
            if (angle < 0)
                angle = -angle;
            if (angle < 1000)
                losDetected = 1;
        }
    }
    else
    {
        hit = 0;
    }
    flagByte = ((EnemyState*)state)->userData2;
    if ((flagByte & 0x40) == 0)
    {
        Sfx_PlayFromObjectLimited((int)obj, SFXTRIG_baddie_blooplaugh3, 2);
        baddieSetMove(obj, state, 2, 1.0f, 0, 0);
        ((EnemyState*)state)->userData2 = (u8)((((EnemyState*)state)->userData2) | 0x40);
        ((EnemyState*)state)->userData1 = 0;
    }
    else if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        u8 mode;
        if ((u8)hit != 0)
        {
            if (((EnemyState*)state)->userData1 != 0)
            {
                ((EnemyState*)state)->userData1 -= 1;
                mode = (u8)(obj)->anim.currentMove;
            }
            else if ((obj)->anim.currentMove != 5 && losDetected)
            {
                mode = 5;
                ((EnemyState*)state)->userData1 =
                    gMagicPlantSeqEntryTable[((EnemyState*)state)->userData2 & 3];
                ((EnemyState*)state)->userData2 =
                    (u8)((*(s8*)&((EnemyState*)state)->userData2 + 1) & 0xc3);
            }
            else
            {
                mode = 4;
                rnd = randomGetRange(1, 2);
                ((EnemyState*)state)->userData1 = rnd;
            }
        }
        else
        {
            rnd = randomGetRange(2, 4);
            mode = rnd;
            if (mode == 2)
            {
                mode = 0;
            }
            else if (mode == 4)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_newtricky_01j);
            }
        }
        baddieSetMove(obj, state, mode, 1.5f, 0, 0);
    }
    if ((obj)->anim.currentMove == 5 && (double)(obj)->anim.currentMoveProgress >= 0.7647 &&
        (double)(obj)->anim.currentMoveProgress < 0.7647 + ((EnemyState*)state)->animPlaySpeed * timeDelta)
    {
        kooshy_spawnProjectile(obj, state);
    }
    else
    {
        *(f32*)(state + 0x324) = *(f32*)(state + 0x324) - timeDelta;
        if (*(f32*)(state + 0x324) <= 0.0f)
        {
            rnd = randomGetRange(0x96, 0x12c);
            *(f32*)(state + 0x324) = (f32)(s32)rnd;
            Sfx_PlayFromObject((int)obj, SFXTRIG_sc_clubswipe);
        }
    }
    magicplantSpawnMovePuffs(obj, state);
}

void kooshy_updateEngaged(GameObject* obj, int state)
{
    ((EnemyState*)state)->userData2 = ((EnemyState*)state)->userData2 & 0xbf;
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0 && (obj)->anim.currentMove != 1)
    {
        Sfx_PlayFromObjectLimited((int)obj, SFXTRIG_baddie_eggsnatch_movelp, 2);
        baddieSetMove(obj, state, 1, 1.0f, 0, 0);
    }
    magicplantSpawnMovePuffs(obj, state);
}

void kooshy_init(int unused, int state)
{
    f32 eventFlagsVal;
    f32 pathStepInit;
    ((EnemyState*)state)->sightRange = 40.0f;
    ((EnemyState*)state)->flags2E4 = 1;
    ((EnemyState*)state)->animPlaySpeed = 0.02f;
    ((EnemyState*)state)->gravity = 0.1f;
    ((EnemyState*)state)->drag = 0.97f;
    ((EnemyState*)state)->moveId0 = 0;
    eventFlagsVal = 1.5f;
    ((EnemyState*)state)->moveSpeedScale0 = eventFlagsVal;
    ((EnemyState*)state)->moveId1 = 7;
    pathStepInit = 1.0f;
    ((EnemyState*)state)->moveSpeedScale1 = pathStepInit;
    ((EnemyState*)state)->moveId2 = 0;
    ((EnemyState*)state)->moveSpeedScale2 = eventFlagsVal;
    ((EnemyState*)state)->userData1 = 0;
    ((EnemyState*)state)->userData2 = 0;
    *(f32*)(state + 0x324) = 150.0f;
    ((EnemyState*)state)->pathStep = pathStepInit;
}

#define FALL_LADDERS_HIT_VOLUME_SLOT 0x18

extern int gWeevilCurveInitData[2];
extern const f64 gWeevilFrozenProgressThreshold;
extern const f32 gWeevilRecoverDurationFrames;
extern const f32 gWeevilApproachDurationFrames;
extern const f32 gWeevilMoveBlendSpeed;
extern const f32 gWeevilZero;
extern const f32 gWeevilCurveInitDistance;
extern const f32 gWeevilOne;
extern const f32 gWeevilHalf;

void weevil_updateWhileFrozen(GameObject* obj, u8* state, int attacker, int msgFlag, int wpad0, int wpad1, Vec* wpad2,
                              int wpad3)
{
    u8 cond = 0;
    int kind = (obj)->anim.currentMove;
    do
    {
        if (kind == 5)
        {
        }
        else if (kind == 4)
        {
        }
        else if (kind == 6)
        {
            if ((double)(obj)->anim.currentMoveProgress < gWeevilFrozenProgressThreshold)
            {
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }

        if (msgFlag != 0xe)
        {
            cond = 1;
        }
    } while (0);

{
    u32 condV = cond;
    if (msgFlag == 0x10)
    {
        if (condV != 0)
        {
            ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x20;
        }
    }
    else if (condV != 0)
    {
        if (((EnemyState*)state)->userData2 == 0)
        {
            ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x8;
            ((EnemyState*)state)->current = 0;
            Sfx_PlayFromObject((int)obj, SFXTRIG_dn_boar1_c_25f);
        }
    }
    else if (msgFlag == 0x11)
    {
        ((EnemyState*)state)->weevil.recoverTimer = gWeevilRecoverDurationFrames;
        ((EnemyState*)state)->weevil.approachTimer = gWeevilApproachDurationFrames;
        baddieSetMove(obj, (int)state, 4, gWeevilMoveBlendSpeed, 0, 3);
        ((EnemyState*)state)->flags2E4 = ((EnemyState*)state)->flags2E4 | 0x10000LL;
        ((EnemyState*)state)->userData2 = 0x3c;
    }
    else
    {
        ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x10;
    }
}
}

void weevil_updateIdle(GameObject* obj, int state)
{
    RomCurveWalker* curve;
    u32 rnd;
    u8 ctr;

    curve = *(RomCurveWalker**)state;
    ((EnemyState*)state)->userData1 = 0;
    ((EnemyState*)state)->weevil.retreatTimer = gWeevilZero;
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_PATH_FOLLOW) != 0)
    {
        if (Curve_AdvanceAlongPath(&curve->curve, ((EnemyState*)state)->pathStep) != 0 ||
            curve->atSegmentEnd != 0)
        {
            if ((*gRomCurveInterface)->goNextPoint(curve) != 0)
            {
                if ((*gRomCurveInterface)
                        ->initCurve(*(RomCurveWalker**)state, (void*)obj, gWeevilCurveInitDistance, gWeevilCurveInitData, -1) != 0)
                {
                    ((EnemyState*)state)->controlFlags =
                        ((EnemyState*)state)->controlFlags & ~(u64)BADDIE_CONTROL_PATH_FOLLOW;
                }
            }
        }
        if (gWeevilZero == ((EnemyState*)state)->weevil.recoverTimer)
        {
            if ((obj)->anim.currentMove == 0)
            {
                baddieTurnTowardPoint(obj, state, curve->posX, curve->posZ, 0x3c, 0);
            }
            if (((EnemyState*)state)->weevil.approachTimer > gWeevilZero)
            {
                f32 zero = gWeevilZero;
                ((EnemyState*)state)->weevil.approachTimer = ((EnemyState*)state)->weevil.approachTimer - timeDelta;
                if (((EnemyState*)state)->weevil.approachTimer <= zero)
                {
                    ((EnemyState*)state)->flags2E4 = ((EnemyState*)state)->flags2E4 & ~(u64)0x10000;
                    ((EnemyState*)state)->weevil.approachTimer = zero;
                }
            }
        }
    }
    if (((EnemyState*)state)->weevil.recoverTimer > gWeevilZero)
    {
        f32 zero = gWeevilZero;
        ((EnemyState*)state)->weevil.recoverTimer = ((EnemyState*)state)->weevil.recoverTimer - timeDelta;
        if (((EnemyState*)state)->weevil.recoverTimer <= zero)
        {
            baddieSetMove(obj, state, 6, gWeevilMoveBlendSpeed, 0, 3);
            ((EnemyState*)state)->weevil.recoverTimer = gWeevilZero;
        }
        else if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            baddieSetMove(obj, state, 5, gWeevilOne, 0, 3);
        }
    }
    else if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        baddieSetMove(obj, state, 0, gWeevilHalf, 0, 3);
    }
    (obj)->anim.rotY = ((EnemyState*)state)->spawnRotY;
    (obj)->anim.rotZ = ((EnemyState*)state)->spawnRotZ;
    ((EnemyState*)state)->weevil.gruntTimer = ((EnemyState*)state)->weevil.gruntTimer - timeDelta;
    if (((EnemyState*)state)->weevil.gruntTimer <= gWeevilZero)
    {
        rnd = randomGetRange(0x3c, 0x78);
        ((EnemyState*)state)->weevil.gruntTimer = (f32)(s32)rnd;
        Sfx_PlayFromObject((int)obj, SFXTRIG_dn_boar1_c_25e);
    }
    ctr = ((EnemyState*)state)->userData2;
    if (ctr != 0)
    {
        ((EnemyState*)state)->userData2--;
    }
}

void weevil_updateEngaged(GameObject* obj, int state)
{
    u8 done;

    ((EnemyState*)state)->weevil.recoverTimer = gWeevilZero;
    done = 0;
    ObjHits_SetHitVolumeSlot(&obj->anim, FALL_LADDERS_HIT_VOLUME_SLOT, 1, -1);
    if (*(void**)(state + 0x340) != 0)
    {
        done = 1;
        ((EnemyState*)state)->weevil.approachTimer = 360.0f;
        ((EnemyState*)state)->weevil.recoverTimer = gWeevilZero;
        if (obj->anim.currentMove != 0)
        {
            baddieSetMove(obj, state, 2, gWeevilHalf, 0, 3);
        }
    }
    if (obj->anim.currentMove != 3)
    {
        baddieTurnTowardPoint(obj, state, ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX,
                    ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ, 0x3c, 0);
    }
    else
    {
        ((EnemyState*)state)->weevil.retreatTimer -= timeDelta;
        if (((EnemyState*)state)->weevil.retreatTimer <= gWeevilZero)
        {
            done = 1;
            ((EnemyState*)state)->weevil.recoverTimer = gWeevilRecoverDurationFrames;
            ((EnemyState*)state)->weevil.approachTimer = gWeevilApproachDurationFrames;
            baddieSetMove(obj, state, 4, gWeevilMoveBlendSpeed, 0, 3);
        }
    }
    if (done != 0)
    {
        ((EnemyState*)state)->flags2E4 |= (u64)0x10000;
    }
    else if (((EnemyState*)state)->userData1 == 0)
    {
        ((EnemyState*)state)->userData1 = 1;
        baddieSetMove(obj, state, 1, 0.35f, 0, 3);
    }
    else if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0 &&
             (baddieSetMove(obj, state, 3, 0.375f, 0, 3),
              gWeevilZero == ((EnemyState*)state)->weevil.retreatTimer))
    {
        ((EnemyState*)state)->weevil.retreatTimer = 50.0f;
        baddieTurnTowardPoint(obj, state, ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX,
                    ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ, 1, 0);
        Sfx_PlayFromObject((int)obj, SFXTRIG_dn_boar1_c_25d);
    }
    obj->anim.rotY = ((EnemyState*)state)->spawnRotY;
    obj->anim.rotZ = ((EnemyState*)state)->spawnRotZ;
    if (((EnemyState*)state)->userData2 != 0)
    {
        ((EnemyState*)state)->userData2 -= 1;
    }
}

void weevil_init(int unused, u8* state)
{
    f32 fz;
    f32 fc;
    ((EnemyState*)state)->sightRange = 40.0f;
    ((EnemyState*)state)->flags2E4 = 173;
    ((EnemyState*)state)->animPlaySpeed = 0.02f;
    ((EnemyState*)state)->gravity = gWeevilOne;
    ((EnemyState*)state)->drag = 0.97f;
    ((EnemyState*)state)->moveId0 = 0;
    fz = 1.5f;
    ((EnemyState*)state)->moveSpeedScale0 = fz;
    ((EnemyState*)state)->moveId1 = 7;
    ((EnemyState*)state)->moveSpeedScale1 = 4.0f;
    ((EnemyState*)state)->moveId2 = 0;
    ((EnemyState*)state)->moveSpeedScale2 = fz;
    fc = gWeevilZero;
    ((EnemyState*)state)->weevil.approachTimer = fc;
    ((EnemyState*)state)->weevil.retreatTimer = fc;
    ((EnemyState*)state)->weevil.recoverTimer = fc;
    ((EnemyState*)state)->userData1 = 0;
    ((EnemyState*)state)->userData2 = 0;
    ((EnemyState*)state)->weevil.gruntTimer = 60.0f;
    ((EnemyState*)state)->pathStep = gWeevilHalf;
}

int gPinPonCurveInitData[2] = {2, 3};

void baddieSpawnWaterRipple(GameObject* obj, EnemyState* state)
{
    f32 mtx[17];
    MatrixTransform stk;
    f32 tx;
    f32 ox;
    f32 tz;

    *(f32*)((u8*)state + 0x330) -= timeDelta;
    if (*(f32*)((u8*)state + 0x330) <= 0.0f)
    {
        *(f32*)((u8*)state + 0x330) = (f32)(s32)randomGetRange(30, 60);
        stk.x = obj->anim.localPosX;
        stk.y = 0.0f;
        stk.z = obj->anim.localPosZ;
        stk.rotX = obj->anim.rotX;
        stk.rotY = 0;
        stk.rotZ = 0;
        stk.scale = 1.0f;
        setMatrixFromObjectPos(mtx, &stk);
        tx = 5.0f + (f32)(s32)randomGetRange(-20, 20) / 10.0f;
        tz = 2.0f + (f32)(s32)randomGetRange(-20, 20) / 10.0f;
        Matrix_TransformPoint(mtx, tx, 0.0f, tz, &tx, &ox, &tz);
        (*gWaterfxInterface)->spawnRipple(tx, *(f32*)((u8*)state + 0x32c), tz, 0, 0.0f, 3);
        if (sqrtf(obj->anim.velocityX * obj->anim.velocityX + obj->anim.velocityZ * obj->anim.velocityZ) > 0.5f)
        {
            Sfx_PlayAtPositionFromObject((int)obj, stk.x, stk.y, stk.z, SFXstaff_proj_putaway);
        }
    }
}

void pinPon_updateWhileFrozen(int obj, EnemyState* state, int unused, int cmd, int wpad0, int wpad1,
                              Vec* wpad2, int wpad3)
{
    GameObject* self = (GameObject*)obj;
    if (cmd == 17 || cmd == 16)
        return;
    if (self->anim.currentMoveProgress > 0.5f)
    {
        state->flags2E8 |= 8;
        Sfx_PlayFromObject(obj, SFXTRIG_en_rfall5_c);
        Sfx_PlayFromObject((int)self, SFXTRIG_wp_iceywindlp16_233);
        state->current = 0;
        state->flags2E4 |= 32;
    }
    else
    {
        state->flags2E8 |= 16;
    }
}

void pinPon_updateIdle(GameObject* obj, int state)
{
    ObjHitsPriorityState* hitState;
    RomCurveWalker* curve;
    u8 rnd;
    f32 vec[3];

    curve = *(RomCurveWalker**)state;
    ((EnemyState*)state)->userData2 = 0;
    hitState = (ObjHitsPriorityState*)(obj)->anim.hitReactState;
    hitState->suppressOutgoingHits = 0;
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_PATH_FOLLOW) != 0)
    {
        if ((Curve_AdvanceAlongPath(&curve->curve, ((EnemyState*)state)->pathStep) != 0 ||
             curve->atSegmentEnd != 0) &&
            (*gRomCurveInterface)->goNextPoint((void*)curve) != 0 &&
            (*gRomCurveInterface)
                    ->initCurve(*(RomCurveWalker**)state, (void*)obj, 700.0f, gPinPonCurveInitData, -1) != 0)
        {
            ((EnemyState*)state)->controlFlags &= ~(u64)BADDIE_CONTROL_PATH_FOLLOW;
        }
        vec[0] = curve->posX - (obj)->anim.localPosX;
        vec[1] = 0.0f;
        vec[2] = curve->posZ - (obj)->anim.localPosZ;
        enemy_steerVelocityToward(obj, (void*)state, vec, 2.0f, 0.1f, 0.1f, 1);
        *(f32*)(state + 0x324) += timeDelta;
        if (*(f32*)(state + 0x324) > 360.0f)
        {
            ((EnemyState*)state)->flags2E4 &= ~(u64)0x10000;
            *(f32*)(state + 0x324) = 0.0f;
        }
    }
    (obj)->anim.rotY =
        -(1024.0f * mathSinfFast(0.19634955f * (f32)(u32)((EnemyState*)state)->userData1) - (f32)(obj)->anim.rotY);
    baddieTurnTowardLookDir(obj, (void*)state, 0xf, 7.5f, 1.0f, 0);
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        if ((obj)->anim.currentMoveProgress < 0.5)
        {
            rnd = randomGetRange(0, 200);
        }
        else
        {
            rnd = randomGetRange(0, 0x3c);
        }
        if (rnd == 0)
        {
            if ((obj)->anim.currentMoveProgress > 0.5)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_kooshy_hit);
                ((EnemyState*)state)->animPlaySpeed = -0.02f;
            }
            else
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_kooshy_death);
                ((EnemyState*)state)->animPlaySpeed = 0.02f;
            }
        }
    }
    ((EnemyState*)state)->userData1 += 1;
    (obj)->anim.rotY =
        1024.0f * mathSinfFast(0.19634955f * (f32)(u32)((EnemyState*)state)->userData1) + (f32)(obj)->anim.rotY;
    baddieSpawnWaterRipple(obj, (EnemyState*)state);
}

#define FIREFLYLANTERN_HIT_VOLUME_SLOT 0xe

extern f32 gFireflyLanternPathStepScale;

void pinPon_updateEngaged(GameObject* obj, int* state)
{
    RomCurveWalker* curve;
    u8 flag;
    f32 dvec[3];
    f32 fval;

    curve = (RomCurveWalker*)*state;
    if (((EnemyState*)state)->controlFlags & 0x80000000U)
    {
        Sfx_PlayFromObject((u32)obj, SFXTRIG_windlift_loop);
    }
    if (((((EnemyState*)state)->controlFlags & 0x2000U) != 0) &&
        ((Curve_AdvanceAlongPath(&curve->curve, 0.0f) != 0 || curve->atSegmentEnd != 0) &&
         ((*gRomCurveInterface)->goNextPoint(curve) != 0)) &&
        ((*gRomCurveInterface)->initCurve((RomCurveWalker*)*state, (void*)obj, 700.0f, (int*)&gPinPonCurveInitData, -1) !=
         0))
    {
        ((EnemyState*)state)->controlFlags &= ~0x2000LL;
    }
    ObjHits_SetHitVolumeSlot(&obj->anim, FIREFLYLANTERN_HIT_VOLUME_SLOT, 1, 0);
    flag = playerGetFlags3F0Bit5((GameObject*)(Obj_GetPlayerObject()));
    dvec[0] = ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX - (obj)->anim.localPosX;
    dvec[1] = 0.0f;
    dvec[2] = ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ - (obj)->anim.localPosZ;
    if (((u32)((EnemyState*)state)->lastHitObject != 0) &&
        ((u32)((EnemyState*)state)->lastHitObject == (u32)Obj_GetPlayerObject()))
    {
        ((EnemyState*)state)->flags2E4 |= 0x10000LL;
        ((EnemyState*)state)->fireflyLantern.trackTimer = 0.0f;
    }
    (obj)->anim.rotY = -(1024.0f * mathSinfFast(0.19634955f * (f32)(u32)((EnemyState*)state)->userData1) -
                         (f32)(obj)->anim.rotY);
    if (flag == 0)
    {
        fval = 0.0f;
        (obj)->anim.velocityX = fval;
        (obj)->anim.velocityZ = fval;
        baddieTurnTowardPoint(obj, (int)state, ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX,
                              ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ, 10, 0);
    }
    else
    {
        enemy_steerVelocityToward(obj, state, dvec, 2.0f, 0.1f, 0.1f, 1);
        baddieTurnTowardLookDir(obj, state, 0xf, 7.5f, 1.0f, 0);
    }
    if (((EnemyState*)state)->controlFlags & 0x40000000U)
    {
        fval = 0.0f;
        if (fval == ((EnemyState*)state)->fireflyLantern.breathTimer)
        {
            if (flag == 0)
            {
                if ((obj)->anim.currentMoveProgress > 0.5f)
                {
                    ((EnemyState*)state)->fireflyLantern.breathTimer = 300.0f;
                    ((EnemyState*)state)->userData2 += 1;
                }
                else
                {
                    ((EnemyState*)state)->fireflyLantern.breathTimer = 120.0f;
                }
            }
            else if ((obj)->anim.currentMoveProgress > 0.5)
            {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_baddie_kooshy_hit);
                ((EnemyState*)state)->animPlaySpeed = -0.02f;
            }
            else
            {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_baddie_kooshy_death);
                ((EnemyState*)state)->animPlaySpeed = 0.02f;
            }
        }
        else
        {
            ((EnemyState*)state)->fireflyLantern.breathTimer -= timeDelta;
            if (((EnemyState*)state)->fireflyLantern.breathTimer <= fval)
            {
                ((EnemyState*)state)->fireflyLantern.breathTimer = fval;
                if ((obj)->anim.currentMoveProgress > 0.5)
                {
                    Sfx_PlayFromObject((u32)obj, SFXTRIG_baddie_kooshy_hit);
                    ((EnemyState*)state)->animPlaySpeed = -0.02f;
                }
                else
                {
                    Sfx_PlayFromObject((u32)obj, SFXTRIG_baddie_kooshy_death);
                    ((EnemyState*)state)->animPlaySpeed = 0.1f;
                }
            }
        }
    }
    ((EnemyState*)state)->userData1 += 1;
    (obj)->anim.rotY = (1024.0f * mathSinfFast(0.19634955f * (f32)(u32)((EnemyState*)state)->userData1) +
                        (f32)(obj)->anim.rotY);
    baddieSpawnWaterRipple(obj, (EnemyState*)state);
}

void pinPon_init(GameObject* obj, void* state)
{
    float fval;
    u32 randVal;

    ((EnemyState*)state)->sightRange = 40.0f;
    ((EnemyState*)state)->flags2E4 = 0x8000009;
    ((EnemyState*)state)->animPlaySpeed = -0.02f;
    ((EnemyState*)state)->gravity = 0.1f;
    ((EnemyState*)state)->drag = 0.97f;
    ((EnemyState*)state)->moveId0 = 0;
    fval = 1.5f;
    ((EnemyState*)state)->moveSpeedScale0 = 1.5f;
    ((EnemyState*)state)->moveId1 = 1;
    ((EnemyState*)state)->moveSpeedScale1 = 1.0f;
    ((EnemyState*)state)->moveId2 = 0;
    ((EnemyState*)state)->moveSpeedScale2 = fval;
    fval = 0.0f;
    ((EnemyState*)state)->fireflyLantern.trackTimer = fval;
    ((EnemyState*)state)->fireflyLantern.breathTimer = fval;
    ((EnemyState*)state)->fireflyLantern.anchorY = obj->anim.localPosY;
    randVal = randomGetRange(0, 0xff);
    ((EnemyState*)state)->userData1 = randVal;
    ((EnemyState*)state)->userData2 = 0;
    ((EnemyState*)state)->fireflyLantern.unk330 = 30.0f;
    randVal = randomGetRange(0x32, 0x4b);
    fval = (f32)(s32)randVal;
    fval = gFireflyLanternPathStepScale * fval;
    ((EnemyState*)state)->pathStep = fval;
}

void fireflyLanternGetTargetAngleAndDistance(GameObject* obj, int state, u16* outAngle, float* outDistance)
{
    f32 targetPos[3];
    f32 tmpA[3];
    f32 vecA[3];
    f32 crossA[3];
    f32 tmpB[3];
    f32 vecB[3];
    f32 crossB[3];
    f32 axisA[3];
    f32 axisB[3];
    f32 objY;
    f32 dxDiff;
    f32 dy;
    f32 d;
    GameObject* targetObj;
    int delta;
    u32 angle;

    vecA[0] = ((EnemyState*)state)->wallPlane.anchorX;
    vecA[1] = ((EnemyState*)state)->wallPlane.anchorY;
    vecA[2] = ((EnemyState*)state)->wallPlane.anchorZ;
    PSVECSubtract((Vec*)vecA, &obj->anim.localPos, (Vec*)tmpA);
    d = PSVECDotProduct((Vec*)tmpA, (Vec*)(state + 0x344));
    vecA[0] = *(f32*)(state + 0x344) * d + obj->anim.localPosX;
    vecA[1] = *(f32*)(state + 0x348) * d + (objY = obj->anim.localPosY);
    vecA[2] = *(f32*)(state + 0x34c) * d + obj->anim.localPosZ;
    axisA[0] = 0.0f;
    axisA[1] = 1.0f;
    axisA[2] = 0.0f;
    PSVECCrossProduct((Vec*)axisA, (Vec*)(state + 0x344), (Vec*)crossA);
    PSVECNormalize((Vec*)crossA, (Vec*)crossA);
    if (0.0f != crossA[0])
    {
        dxDiff = (obj->anim.localPosX - ((EnemyState*)state)->wallPlane.anchorX) / crossA[0];
    }
    else
    {
        dxDiff = (obj->anim.localPosZ - ((EnemyState*)state)->wallPlane.anchorZ) / crossA[2];
    }
    targetObj = ((EnemyState*)state)->trackedObj;
    targetPos[0] = targetObj->anim.localPosX;
    targetPos[1] = gFireflyLanternTargetHeightOffset + targetObj->anim.localPosY;
    targetPos[2] = targetObj->anim.localPosZ;
    vecB[0] = ((EnemyState*)state)->wallPlane.anchorX;
    vecB[1] = ((EnemyState*)state)->wallPlane.anchorY;
    vecB[2] = ((EnemyState*)state)->wallPlane.anchorZ;
    PSVECSubtract((Vec*)vecB, (Vec*)targetPos, (Vec*)tmpB);
    d = PSVECDotProduct((Vec*)tmpB, (Vec*)(state + 0x344));
    vecB[0] = *(f32*)(state + 0x344) * d + targetPos[0];
    vecB[1] = *(f32*)(state + 0x348) * d + (dy = targetPos[1]);
    vecB[2] = *(f32*)(state + 0x34c) * d + targetPos[2];
    axisB[0] = 0.0f;
    axisB[1] = 1.0f;
    axisB[2] = 0.0f;
    PSVECCrossProduct((Vec*)axisB, (Vec*)(state + 0x344), (Vec*)crossB);
    PSVECNormalize((Vec*)crossB, (Vec*)crossB);
    if (0.0f != crossB[0])
    {
        d = (targetPos[0] - ((EnemyState*)state)->wallPlane.anchorX) / crossB[0];
    }
    else
    {
        d = (targetPos[2] - ((EnemyState*)state)->wallPlane.anchorZ) / crossB[2];
    }
    dxDiff = dxDiff - d;
    dy = objY - dy;
    angle = getAngle(-dy, dxDiff) & 0xffff;
    delta = angle - (obj->anim.rotY & 0xffff);
    if (delta > 0x8000)
    {
        delta = delta - 0xffff;
    }
    if (delta < -0x8000)
    {
        delta = delta + 0xffff;
    }
    if (delta < 0)
    {
        delta = -delta;
    }
    *outAngle = delta & 0xffff;
    *outDistance = sqrtf(dxDiff * dxDiff + dy * dy);
}

u32 fireflyLanternSteerTowardTarget(short* obj, int state, u32 turnTime, f32 maxDistance)
{
    f32 moveTarget[3];
    f32 moveDelta[3];
    f32 targetPos[3];
    f32 tmpA[3];
    f32 vecA[3];
    f32 crossA[3];
    f32 tmpB[3];
    f32 vecB[3];
    f32 crossB[3];
    f32 axisA[3];
    f32 axisB[3];
    f32 objY;
    f32 targetY;
    f32 dy;
    f32 dxA;
    f32 dxDiff;
    f32 d;
    f32 turnStep;
    s16 rot;
    GameObject* targetObj;
    int delta;
    int angleStep;
    u32 angle;
    GameObject* o = (GameObject*)obj;
    EnemyState* fs = (EnemyState*)state;

    vecA[0] = fs->wallPlane.anchorX;
    vecA[1] = fs->wallPlane.anchorY;
    vecA[2] = fs->wallPlane.anchorZ;
    PSVECSubtract((Vec*)vecA, (Vec*)(obj + 6), (Vec*)tmpA);
    d = PSVECDotProduct((Vec*)tmpA, (Vec*)fs->wallPlane.normal);
    vecA[0] = fs->wallPlane.normal[0] * d + o->anim.localPosX;
    vecA[1] = fs->wallPlane.normal[1] * d + (objY = o->anim.localPosY);
    vecA[2] = fs->wallPlane.normal[2] * d + o->anim.localPosZ;
    axisA[0] = 0.0f;
    axisA[1] = 1.0f;
    axisA[2] = 0.0f;
    PSVECCrossProduct((Vec*)axisA, (Vec*)fs->wallPlane.normal, (Vec*)crossA);
    PSVECNormalize((Vec*)crossA, (Vec*)crossA);
    if (0.0f != crossA[0])
    {
        dxA = (o->anim.localPosX - fs->wallPlane.anchorX) / crossA[0];
    }
    else
    {
        dxA = (o->anim.localPosZ - fs->wallPlane.anchorZ) / crossA[2];
    }
    targetObj = (GameObject*)(((EnemyState*)state)->trackedObj);
    targetPos[0] = targetObj->anim.localPosX;
    targetPos[1] = gFireflyLanternTargetHeightOffset + targetObj->anim.localPosY;
    targetPos[2] = targetObj->anim.localPosZ;
    vecB[0] = fs->wallPlane.anchorX;
    vecB[1] = fs->wallPlane.anchorY;
    vecB[2] = fs->wallPlane.anchorZ;
    PSVECSubtract((Vec*)vecB, (Vec*)targetPos, (Vec*)tmpB);
    d = PSVECDotProduct((Vec*)tmpB, (Vec*)fs->wallPlane.normal);
    vecB[0] = fs->wallPlane.normal[0] * d + targetPos[0];
    vecB[1] = fs->wallPlane.normal[1] * d + (targetY = targetPos[1]);
    vecB[2] = fs->wallPlane.normal[2] * d + targetPos[2];
    axisB[0] = 0.0f;
    axisB[1] = 1.0f;
    axisB[2] = 0.0f;
    PSVECCrossProduct((Vec*)axisB, (Vec*)fs->wallPlane.normal, (Vec*)crossB);
    PSVECNormalize((Vec*)crossB, (Vec*)crossB);
    if (0.0f != crossB[0])
    {
        d = (targetPos[0] - fs->wallPlane.anchorX) / crossB[0];
    }
    else
    {
        d = (targetPos[2] - fs->wallPlane.anchorZ) / crossB[2];
    }
    dxDiff = dxA - d;
    dy = objY - targetY;
    angle = getAngle(-dy, dxDiff) & 0xffff;
    rot = o->anim.rotY;
    delta = angle - (rot & 0xffff);
    if (delta > 0x8000)
    {
        delta = delta - 0xffff;
    }
    if (delta < -0x8000)
    {
        delta = delta + 0xffff;
    }
    turnStep = timeDelta / (f32)(turnTime & 0xffff);
    if (turnStep > 1.0f)
    {
        turnStep = 1.0f;
    }
    angleStep = (int)((f32)delta * turnStep);
    *obj = (s16)(rot + angleStep);
    o->anim.rotZ = 0x4000;
    o->anim.rotY = *obj;
    *obj = getAngle(fs->wallPlane.normal[2], -fs->wallPlane.normal[0]);
    turnStep = sqrtf(dxDiff * dxDiff + dy * dy);
    if (turnStep > maxDistance)
    {
        f32 ratio = 1.0f / turnStep;
        dxDiff = maxDistance * (dxDiff * ratio);
        dy = maxDistance * (dy * ratio);
    }
    dxA -= dxDiff;
    turnStep = objY - dy;
    wallPlaneClampMoveTarget(moveTarget, &fs->wallPlane, dxA, turnStep);
    PSVECSubtract((Vec*)moveTarget, (Vec*)(obj + 6), (Vec*)moveDelta);
    objMove((GameObject*)obj, moveDelta[0], moveDelta[1], moveDelta[2]);
    turnStep = lbl_803E2A00;
    o->anim.velocityX = turnStep;
    o->anim.velocityY = turnStep;
    o->anim.velocityZ = turnStep;
    if (angleStep < 0)
    {
        angleStep = -angleStep;
    }
    return angleStep & 0xffff;
}

int gWbCurveInitData[2] = {2, 3};

/* object-type id of the pollen-spit projectile spawned by spittingEbaSpawnPollen
 * (see file docblock). */
#define DUSTER_CHILD_OBJ_POLLEN_SPIT 0x47b
#define DUSTER_HIT_VOLUME_SLOT       10

extern f32 gDusterWallProbeOffsets[];

void wallPlaneClampMoveTarget(float* outPos, WallPlaneState* plane, float lateral, float height)
{
    float hi;
    float lo;
    float sideAxis[3];
    float up[3];
    float upConst;
    float scale;

    hi = plane->boundMin - 15.0f;
    if (height > hi)
    {
        height = hi;
    }
    else
    {
        lo = (50.0f) + plane->anchorY;
        if (height < lo)
        {
            height = lo;
        }
    }
    if (plane->axisLimit > 0.0f)
    {
        hi = plane->axisLimit - 15.0f;
        lo = 15.0f;
    }
    else
    {
        hi = -15.0f;
        lo = 15.0f + plane->axisLimit;
    }
    if (lateral > hi)
    {
        lateral = hi;
    }
    else
    {
        if (lateral < lo)
        {
            lateral = lo;
        }
    }
    outPos[1] = height;
    upConst = 0.0f;
    up[0] = upConst;
    up[1] = 1.0f;
    up[2] = upConst;
    PSVECCrossProduct((Vec*)up, (Vec*)plane->normal, (Vec*)sideAxis);
    PSVECNormalize((Vec*)sideAxis, (Vec*)sideAxis);
    *outPos = lateral * sideAxis[0] + plane->anchorX;
    outPos[2] = lateral * sideAxis[2] + plane->anchorZ;
    scale = (2.0f);
    *outPos = scale * plane->normal[0] + *outPos;
    outPos[1] = scale * plane->normal[1] + outPos[1];
    outPos[2] = scale * plane->normal[2] + outPos[2];
}

void rachnopFindWallPlane(GameObject* obj, int state)
{
    u8 didHit;
    float* probeOffsets;
    int i;
    f32 dot;
    float maxv[3];
    float minv[3];
    float sideAxis0[3];
    float cv[3];
    float av[3];
    float toAnchor[3];
    float bv[3];
    float sideAxis[3];
    float dv[3];
    float hit[18];

    didHit = 0;
    probeOffsets = gDusterWallProbeOffsets;
    for (i = 0; didHit == 0 && i < 4; i++)
    {
        maxv[0] = obj->anim.localPosX + probeOffsets[i * 2 + 0];
        maxv[1] = obj->anim.localPosY;
        maxv[2] = obj->anim.localPosZ + probeOffsets[i * 2 + 1];
        minv[0] = obj->anim.localPosX - probeOffsets[i * 2 + 0];
        minv[1] = obj->anim.localPosY;
        minv[2] = obj->anim.localPosZ - probeOffsets[i * 2 + 1];
        didHit = objBboxFn_800640cc(maxv, minv, 0.0f, 3, (TrackBBoxHit*)hit,
                                    obj, 5, 3, 0xff, 0);
    }
    if (didHit != 0)
    {
        obj->anim.localPosX = (hit[17] - (15.0f)) * ((minv[0] - maxv[0]) / (50.0f)) + maxv[0];
        obj->anim.localPosZ = (hit[17] - (15.0f)) * ((minv[2] - maxv[2]) / (50.0f)) + maxv[2];
        *(float*)(state + DUSTER_WALL_NORMAL_X_OFFSET) = hit[7];
        *(float*)(state + DUSTER_WALL_NORMAL_Y_OFFSET) = hit[8];
        *(float*)(state + DUSTER_WALL_NORMAL_Z_OFFSET) = hit[9];
        ((EnemyState*)state)->wallPlane.normalW = hit[10];
        ((EnemyState*)state)->wallPlane.anchorY = (hit[3] > hit[4]) ? hit[3] : hit[4];
        ((EnemyState*)state)->wallPlane.boundMin = (hit[15] < hit[16]) ? hit[15] : hit[16];
        av[0] = 0.0f;
        av[1] = 1.0f;
        av[2] = 0.0f;
        PSVECCrossProduct((Vec*)av, (Vec*)(state + DUSTER_WALL_PLANE_OFFSET), (Vec*)sideAxis0);
        PSVECNormalize((Vec*)sideAxis0, (Vec*)sideAxis0);
        ((EnemyState*)state)->wallPlane.anchorX = hit[1];
        ((EnemyState*)state)->wallPlane.anchorZ = hit[5];
        cv[0] = hit[2];
        cv[2] = hit[6];
        bv[0] = ((EnemyState*)state)->wallPlane.anchorX;
        bv[1] = ((EnemyState*)state)->wallPlane.anchorY;
        bv[2] = ((EnemyState*)state)->wallPlane.anchorZ;
        PSVECSubtract((Vec*)bv, (Vec*)cv, (Vec*)toAnchor);
        dot = PSVECDotProduct((Vec*)toAnchor, (Vec*)(state + DUSTER_WALL_PLANE_OFFSET));
        bv[0] = *(float*)(state + DUSTER_WALL_NORMAL_X_OFFSET) * dot + cv[0];
        bv[1] = *(float*)(state + DUSTER_WALL_NORMAL_Y_OFFSET) * dot + cv[1];
        bv[2] = *(float*)(state + DUSTER_WALL_NORMAL_Z_OFFSET) * dot + cv[2];
        dv[0] = 0.0f;
        dv[1] = 1.0f;
        dv[2] = 0.0f;
        PSVECCrossProduct((Vec*)dv, (Vec*)(state + DUSTER_WALL_PLANE_OFFSET), (Vec*)sideAxis);
        PSVECNormalize((Vec*)sideAxis, (Vec*)sideAxis);
        if (0.0f != sideAxis[0])
        {
            ((EnemyState*)state)->wallPlane.axisLimit =
                (cv[0] - ((EnemyState*)state)->wallPlane.anchorX) / sideAxis[0];
        }
        else
        {
            ((EnemyState*)state)->wallPlane.axisLimit =
                (cv[2] - ((EnemyState*)state)->wallPlane.anchorZ) / sideAxis[2];
        }
        ((EnemyState*)state)->userData1 = 1;
    }
}

f32 gDusterWallProbeOffsets[] = {
    50.0f, 0.0f, -50.0f, 0.0f, 0.0f, 50.0f, 0.0f, -50.0f,
};

u8 gDusterEbaMoveTable[] = {
    0x3F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x03, 0x03, 0x00, 0x40, 0x20, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0xCC, 0xCC, 0xCD, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x07, 0x07, 0x00, 0x40, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x08, 0x08, 0x08, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
    0x07, 0x07, 0x07, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x06, 0x05, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x04, 0x07, 0x07, 0x07, 0x00, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08, 0x08, 0x08,
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* baddie-AI tables referenced via extern by firecrawler.c; owned here by link order */

u8 gRedEyeLocomotionMoves[0x30] = {
    0x40, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x0E, 0x00, 0x03, 0x00, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x01, 0x00, 0x03, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00,
};

u8 gRedEyeRandomMoves[0x54] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00,
    0x0F, 0x00, 0x00, 0x12, 0x00, 0x08, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x14, 0x00,
    0x08, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x14, 0x00, 0x08, 0x00, 0x40, 0xA0, 0x00,
    0x00, 0x00, 0x0F, 0x00, 0x00, 0x13, 0x00, 0x03, 0x00, 0x3F, 0xA6, 0x66, 0x66, 0x00, 0x0F, 0x00, 0x00,
    0x10, 0x00, 0x08, 0x00, 0x3F, 0xA6, 0x66, 0x66, 0x00, 0x0F, 0x00, 0x00, 0x11, 0x00, 0x08, 0x00,
};

u8 gRedEyeDefaultMoveChain[0x48] = {
    0x3F, 0xC0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0D, 0x01, 0x03, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x0F,
    0x00, 0x00, 0x0F, 0x02, 0x03, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x0F, 0x03, 0x03, 0x00,
    0x40, 0x60, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x14, 0x04, 0x08, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x0F,
    0x00, 0x00, 0x0F, 0x05, 0x03, 0x00, 0x40, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x13, 0x01, 0x03, 0x00,
};

u8 gRedEyeMoveChain[0x60] = {
    0x40, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x14, 0x01, 0x08, 0x00, 0x3F, 0x80, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x00, 0x02, 0x02, 0x03, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
    0x04, 0x03, 0x03, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x02, 0x04, 0x03, 0x00,
    0x3F, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x05, 0x05, 0x03, 0x00, 0x3F, 0xC0, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x00, 0x03, 0x06, 0x03, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
    0x06, 0x07, 0x03, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x04, 0x00, 0x03, 0x00,
};

u8 gRedEyeOctantMoves[0x60] = {
    0x3F, 0xA6, 0x66, 0x66, 0x00, 0x0F, 0x00, 0x00, 0x12, 0x00, 0x08, 0x00, 0x3F, 0xA6, 0x66, 0x66,
    0x00, 0x0F, 0x00, 0x00, 0x10, 0x00, 0x08, 0x00, 0x3F, 0xA6, 0x66, 0x66, 0x00, 0x0F, 0x00, 0x00,
    0x10, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xA6, 0x66, 0x66,
    0x00, 0x0F, 0x00, 0x00, 0x11, 0x00, 0x08, 0x00, 0x3F, 0xA6, 0x66, 0x66, 0x00, 0x0F, 0x00, 0x00,
    0x11, 0x00, 0x08, 0x00, 0x3F, 0xA6, 0x66, 0x66, 0x00, 0x0F, 0x00, 0x00, 0x12, 0x00, 0x08, 0x00,
};

u8 gRedEyeMoveHitVolumes[0xC0] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x18, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x18, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x04, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x18, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x07, 0x02, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x0D, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x18, 0x0F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x10, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x11, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x18, 0x12, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x13, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x15, 0x01, 0x00, 0x00,
};

u8 gRedEyeHitReactionSeq[0x70] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0B, 0x07, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x3E, 0xCC, 0xCC, 0xCD, 0x00, 0x00,
    0x00, 0x0B, 0x0A, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x08,
    0x03, 0x05, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x0B, 0x03, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x03, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x09, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00,
    0x06, 0x3F, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

u8 gCrawlerDefaultMoveEventFx[0x9C] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xA0, 0x03, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, 0xA0, 0x04, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x9C, 0x05, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x9D, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x08, 0x00, 0x00, 0x20,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x41, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x9E, 0x0B, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x9F, 0x0C, 0x00, 0x00, 0x00,
};

u8 gFireCrawlerLocomotionMoves[0x30] = {
    0x40, 0x40, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x3F, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x03, 0x00, 0x03, 0x00, 0x3F, 0x19, 0x99, 0x9A, 0x00, 0x01, 0x00, 0x00, 0x04, 0x00, 0x03, 0x00,
};

u8 gFireCrawlerOctantMoves[0x60] = {
    0x40, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x3F, 0x99, 0x99, 0x9A,
    0x00, 0x0F, 0x00, 0x00, 0x06, 0x00, 0x08, 0x00, 0x3F, 0x99, 0x99, 0x9A, 0x00, 0x0F, 0x00, 0x00,
    0x06, 0x00, 0x08, 0x00, 0x3F, 0x99, 0x99, 0x9A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3F, 0x99, 0x99, 0x9A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x99, 0x99, 0x9A,
    0x00, 0x0F, 0x00, 0x00, 0x07, 0x00, 0x08, 0x00, 0x3F, 0x99, 0x99, 0x9A, 0x00, 0x0F, 0x00, 0x00,
    0x07, 0x00, 0x08, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00,
};

u8 gFireCrawlerRandomMoves[0x24] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x0F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
};

u8 gFireCrawlerMoveChain[0x24] = {
    0x40, 0x19, 0x99, 0x9A, 0x00, 0x0F, 0x00, 0x00, 0x05, 0x01, 0x03, 0x00, 0x3F, 0xB3, 0x33, 0x33, 0x00, 0x0F,
    0x00, 0x00, 0x09, 0x02, 0x03, 0x00, 0x40, 0x80, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x0A, 0x00, 0x03, 0x00,
};

u8 gFireCrawlerMoveHitVolumes[0x9C] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x09, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x03, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x09, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x06, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x09, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x09, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x0A, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x1F, 0x0B, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x0C, 0x02, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x0D, 0x02, 0x00, 0x00,
};

u8 gFireCrawlerHitReactionSeq[0x50] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x0B, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06,
    0x3F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x0C, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x3F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x0D, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x3F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
};

u8 gFireCrawlerMoveEventFx[0x9C] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0xAB, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xAB, 0x02, 0x00, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00,
};

u8 gShadowHunterLocomotionMoves[0x30] = {
    0x40, 0x80, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x3F, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x04, 0x00, 0x03, 0x00, 0x3F, 0x19, 0x99, 0x9A, 0x00, 0x01, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00,
};

u8 gShadowHunterOctantMoves[0x60] = {
    0x40, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x3F, 0xC0, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x00, 0x06, 0x00, 0x08, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
    0x06, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xC0, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x00, 0x07, 0x00, 0x08, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
    0x07, 0x00, 0x08, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00,
};

u8 gShadowHunterRandomMoves[0x30] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x40, 0x80, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x40, 0x80, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
    0x01, 0x00, 0x08, 0x00, 0x40, 0x80, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x02, 0x00, 0x08, 0x00,
};

u8 gShadowHunterMoveChain[0x24] = {
    0x3F, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x05, 0x01, 0x03, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x0F,
    0x00, 0x00, 0x05, 0x02, 0x03, 0x00, 0x3F, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00,
};

u8 gShadowHunterDefaultMoveChain[0x3C] = {
    0x3F, 0x80, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x09, 0x01, 0x03, 0x02, 0x3F, 0x80, 0x00,
    0x00, 0x00, 0x0F, 0x00, 0x00, 0x0A, 0x00, 0x03, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x00, 0x0F,
    0x00, 0x00, 0x0B, 0x03, 0x03, 0x03, 0x3F, 0x80, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x0C,
    0x04, 0x03, 0x04, 0x3F, 0x80, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x0D, 0x00, 0x03, 0x00,
};

u8 gShadowHunterMoveHitVolumes[0x30] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0A, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
    0x0A, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0D, 0x01, 0x00, 0x00,
};

u8 gShadowHunterHitReactionSeq[0x40] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x0E, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06,
    0x3F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x0F, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06,
    0x3F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

extern f32 gDusterWallProbeOffsets[];

void rachnopUpdateWhileFrozen(GameObject* obj, u8* state, int unused, int eventKind, int wpad0, int wpad1, Vec* wpad2, int wpad3)
{
    if (eventKind == 0x10)
    {
        ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x20;
    }
    else if (eventKind != 0x11)
    {
        ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 8;
        Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_zyck_lash_254);
        ((EnemyState*)state)->current = 0;
    }
    return;
}

void rachnopUpdateIdle(GameObject* obj, int state)
{
    int cond;

    if (((EnemyState*)state)->userData1 == 0)
    {
        rachnopFindWallPlane(obj, state);
    }
    else
    {
        if ((((GameObject*)((EnemyState*)state)->trackedObj)->anim.classId == 1) &&
            (cond = fn_80295CBC((GameObject*)(((EnemyState*)state)->trackedObj)), cond != 0))
        {
            ((EnemyState*)state)->flags2E4 = ((EnemyState*)state)->flags2E4 & ~0x10000LL;
        }
        if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_id_253);
            Baddie_SetMove((int)obj, state, 2, 1.0f, 0, 0);
        }
    }
    return;
}

void rachnopUpdateApproach(GameObject* obj, int state)
{
    int cond;

    if (((EnemyState*)state)->userData1 == 0)
    {
        rachnopFindWallPlane(obj, state);
    }
    else if ((((GameObject*)((EnemyState*)state)->trackedObj)->anim.classId == 1) &&
             (cond = fn_80295CBC((GameObject*)(((EnemyState*)state)->trackedObj)), cond != 0))
    {
        fireflyLanternSteerTowardTarget((short*)obj, state, 0x19, (double)(0.5f));
        if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            Baddie_SetMove((int)obj, state, 0, (0.5f), 0, 0);
            Sfx_PlayFromObject((u32)obj, SFXTRIG_id_252);
        }
    }
    else
    {
        ((EnemyState*)state)->flags2E4 = ((EnemyState*)state)->flags2E4 | 0x10000LL;
    }
    return;
}

void rachnopUpdateAttack(GameObject* obj, int state)
{
    short move;
    int cond;
    u16 outIds[2];
    float outVec[3];

    if (((EnemyState*)state)->userData1 == 0)
    {
        rachnopFindWallPlane(obj, state);
    }
    else if ((((GameObject*)((EnemyState*)state)->trackedObj)->anim.classId == 1) &&
             (cond = fn_80295CBC((GameObject*)(((EnemyState*)state)->trackedObj)), cond != 0))
    {
        ObjHits_SetHitVolumeSlot(&obj->anim, DUSTER_HIT_VOLUME_SLOT, 1, 0);
        move = obj->anim.currentMove;
        if (move == 3)
        {
            fireflyLanternSteerTowardTarget((short*)obj, state, 0x19, (double)0.0f);
        }
        else if ((move == 0) || (move == 1))
        {
            fireflyLanternSteerTowardTarget((short*)obj, state, 0x19, (double)0.5f);
        }
        fireflyLanternGetTargetAngleAndDistance(obj, state, outIds, outVec);
        if (((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0) ||
            ((outIds[0] < 0x5dc && (obj->anim.currentMove != 1))))
        {
            if (outIds[0] < 0x5dc)
            {
                Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_251);
                Baddie_SetMove((int)obj, state, 1, 0.5f, 0, 0);
            }
            else
            {
                Baddie_SetMove((int)obj, state, 3, 0.5f, 0, 0);
            }
        }
    }
    else
    {
        ((EnemyState*)state)->flags2E4 = ((EnemyState*)state)->flags2E4 | 0x10000LL;
    }
    return;
}

void rachnopInit(u32 unused, int state)
{
    float fa;
    float fb;

    ((EnemyState*)state)->sightRange = (25.0f);
    ((EnemyState*)state)->flags2E4 = 1;
    fa = (0.1f);
    ((EnemyState*)state)->animPlaySpeed = (0.1f);
    ((EnemyState*)state)->gravity = fa;
    ((EnemyState*)state)->drag = (0.97f);
    ((EnemyState*)state)->moveId0 = 0;
    fb = 1.5f;
    ((EnemyState*)state)->moveSpeedScale0 = 1.5f;
    ((EnemyState*)state)->moveId1 = 4;
    fa = 1.0f;
    ((EnemyState*)state)->moveSpeedScale1 = 1.0f;
    ((EnemyState*)state)->moveId2 = 0;
    ((EnemyState*)state)->moveSpeedScale2 = fb;
    ((EnemyState*)state)->duster.phaseTimer = 0.0f;
    ((EnemyState*)state)->userData1 = 0;
    ((EnemyState*)state)->userData2 = 0;
    ((EnemyState*)state)->pathStep = fa;
    return;
}

extern const f32 gDusterDayStartSeconds;
extern const f32 gDusterDayEndSeconds;

void spittingEbaSpawnPollen(GameObject* obj, int state)
{
    u32 loadLocked;
    int ref;
    u16* setup;
    f32 spd;
    f32 t;
    f32 dx;
    f32 dz;
    f32 a[3];
    f32 b[3];
    float velXZ;
    float cosVal;
    float velY;
    float cosPitch;

    loadLocked = Obj_IsLoadingLocked();
    if ((loadLocked & 0xff) != 0)
    {
        a[0] = obj->anim.localPosX;
        a[1] = 15.0f + obj->anim.localPosY;
        a[2] = obj->anim.localPosZ;
        ref = (int)((EnemyState*)state)->trackedObj;
        b[0] = ((GameObject*)ref)->anim.localPosX;
        b[1] = 30.0f + ((GameObject*)ref)->anim.localPosY;
        b[2] = ((GameObject*)ref)->anim.localPosZ;
        spd = (3.25f) * ((0.02f) * (f32)(int)randomGetRange(-10, 10) + (1.0f));
        ref = pinponspike_calculateLaunchAngle(a, b, spd, 1, (0.045f));
        angleToVec2Precise(ref, &cosVal, &velXZ);
        velXZ = velXZ * spd;
        cosVal = cosVal * spd;
        dx = b[0] - obj->anim.localPosX;
        dz = b[2] - obj->anim.localPosZ;
        if (0.0f != dz)
        {
            ref = getAngle(dx, dz);
            angleToVec2Precise(ref, &cosPitch, &velY);
            t = velXZ;
            velY = velY * t;
            velXZ = t * cosPitch;
        }
        else
        {
            velY = 0.0f;
        }
        setup = (u16*)Obj_AllocObjectSetup(0x24, DUSTER_CHILD_OBJ_POLLEN_SPIT);
        ((ObjPlacement*)setup)->posX = a[0];
        ((ObjPlacement*)setup)->posY = a[1];
        ((ObjPlacement*)setup)->posZ = a[2];
        ((ObjPlacement*)setup)->color[0] = 1;
        ((ObjPlacement*)setup)->color[1] = 1;
        ((ObjPlacement*)setup)->color[2] = 0xff;
        ((ObjPlacement*)setup)->color[3] = 0xff;
        ref = (int)Obj_SetupObject((ObjPlacement*)setup, 5, -1, -1, 0);
        if ((void*)ref != NULL)
        {
            ((GameObject*)ref)->anim.velocityX = velXZ;
            ((GameObject*)ref)->anim.velocityY = cosVal;
            ((GameObject*)ref)->anim.velocityZ = velY;
            *(u32*)&((GameObject*)ref)->ownerObj = (u32)obj;
            Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_mika_cackle);
        }
    }
    return;
}

void spittingEbaUpdateTimeOfDay(int obj, int state)
{
    u8 isDaytime;
    float timeInfo[4];

    (*gSkyInterface)->getTimeOfDay(timeInfo);
    if ((timeInfo[0] >= gDusterDayStartSeconds) && (timeInfo[0] <= gDusterDayEndSeconds))
    {
        isDaytime = 1;
    }
    else
    {
        isDaytime = 0;
    }
    if ((isDaytime != 0) && (((EnemyState*)state)->userData1 == 0))
    {
        ((EnemyState*)state)->userData1 = 1;
        ((EnemyState*)state)->flags2E4 = ((EnemyState*)state)->flags2E4 | 0x10000LL;
        Baddie_SetMove(obj, state, 1, 2.0f, 0, 0);
    }
    else if ((isDaytime == 0) && (((EnemyState*)state)->userData1 == 2))
    {
        ((EnemyState*)state)->userData1 = 1;
        ((EnemyState*)state)->flags2E4 = ((EnemyState*)state)->flags2E4 | 0x10000LL;
        Baddie_SetMove(obj, state, 3, 2.0f, 0, 0);
    }
    return;
}

void spittingEbaUpdateWhileFrozen(int obj, u8* state, int unused1, int eventKind, int unused2, int damage, Vec* wpad0, int wpad1)
{
    if (eventKind == 0x10)
    {
        ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x20;
    }
    else if (eventKind == 0x11)
    {
        if ((((EnemyState*)state)->userData1 == 2) && (((GameObject*)obj)->anim.currentMove != 5))
        {
            Baddie_SetMove(obj, (int)state, 5, 3.0f, 0, 0);
        }
    }
    else if ((((GameObject*)obj)->anim.currentMove == 5) || (((GameObject*)obj)->anim.currentMove == 4))
    {
        if (damage > (int)(u32)((EnemyState*)state)->current)
        {
            ((EnemyState*)state)->current = 0;
            Sfx_PlayFromObject(obj, SFXTRIG_baddie_zyck_strike);
            Sfx_PlayFromObject(obj, SFXTRIG_stftest);
        }
        else
        {
            ((EnemyState*)state)->current = ((EnemyState*)state)->current - damage;
            Sfx_PlayFromObject(obj, SFXTRIG_baddie_kooshy_call);
            Sfx_PlayFromObject(obj, SFXTRIG_stftest);
        }
        ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 8;
    }
    else
    {
        ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x10;
        Sfx_PlayFromObject(obj, SFXTRIG_mv_ladderslide16_250);
    }
    return;
}

void spittingEbaUpdateIdle(GameObject* obj, int state)
{
    ((EnemyState*)state)->duster.phaseTimer = 0.0f;
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        if (((EnemyState*)state)->userData1 == 1)
        {
            if ((obj)->anim.currentMove == 1)
            {
                ((EnemyState*)state)->userData1 = 2;
                ((EnemyState*)state)->flags2E4 = ((EnemyState*)state)->flags2E4 & ~0x10000LL;
            }
            else if ((obj)->anim.currentMove == 3)
            {
                ((EnemyState*)state)->userData1 = 0;
                ((EnemyState*)state)->flags2E4 = ((EnemyState*)state)->flags2E4 | 0x10000LL;
                Baddie_SetMove(obj, state, 0, (1.0f), 0, 0);
            }
        }
        else if ((((EnemyState*)state)->userData1 == 2) && ((obj)->anim.currentMove != 2))
        {
            Baddie_SetMove(obj, state, 2, (1.0f), 0, 0);
        }
    }
    spittingEbaUpdateTimeOfDay((int)obj, state);
    return;
}

void spittingEbaUpdateEngaged(GameObject* obj, int state)
{
    u8 timerExpired;

    timerExpired = 0;
    ((EnemyState*)state)->duster.phaseTimer = ((EnemyState*)state)->duster.phaseTimer - timeDelta;
    if (((EnemyState*)state)->duster.phaseTimer <= 0.0f)
    {
        timerExpired = 1;
        ((EnemyState*)state)->duster.phaseTimer = 0.0f;
    }
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        if (obj->anim.currentMove == 4)
        {
            spittingEbaSpawnPollen(obj, state);
            ((EnemyState*)state)->duster.phaseTimer = 180.0f;
            Baddie_SetMove(obj, state, 5, (1.0f), 0, 0);
        }
        else if ((obj->anim.currentMove == 5) && (timerExpired))
        {
            Baddie_SetMove(obj, state, 6, (1.0f), 0, 0);
            Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_kooshy_death);
        }
        else if (obj->anim.currentMove == 6)
        {
            Baddie_SetMove(obj, state, 2, (1.0f), 0, 0);
            ((EnemyState*)state)->duster.phaseTimer = 180.0f;
        }
        else if ((obj->anim.currentMove == 2) && (timerExpired) &&
                 ((((EnemyState*)state)->controlFlags & 0x4000000) != 0))
        {
            Baddie_SetMove(obj, state, 4, (1.0f), 0, 0);
            Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_kooshy_hit);
        }
    }
    spittingEbaUpdateTimeOfDay((int)obj, state);
    return;
}

void spittingEbaInit(u32 unused, int state)
{
    float fa;
    float fb;

    ((EnemyState*)state)->sightRange = 40.0f;
    ((EnemyState*)state)->flags2E4 = 1;
    ((EnemyState*)state)->animPlaySpeed = (0.02f);
    ((EnemyState*)state)->gravity = 0.1f;
    ((EnemyState*)state)->drag = 0.97f;
    ((EnemyState*)state)->moveId0 = 0;
    fb = 1.5f;
    ((EnemyState*)state)->moveSpeedScale0 = 1.5f;
    ((EnemyState*)state)->moveId1 = 7;
    fa = (1.0f);
    ((EnemyState*)state)->moveSpeedScale1 = (1.0f);
    ((EnemyState*)state)->moveId2 = 0;
    ((EnemyState*)state)->moveSpeedScale2 = fb;
    ((EnemyState*)state)->userData1 = 0;
    ((EnemyState*)state)->duster.phaseTimer = 0.0f;
    ((EnemyState*)state)->pathStep = fa;
    return;
}

extern int gWbCurveInitData[2];

void wbUpdateWhileFrozen(int obj, u8* state, int unused, int eventKind, int wpad0, int wpad1, Vec* wpad2, int wpad3)
{
    if (eventKind != 0x11)
    {
        if (eventKind == 0x10)
        {
            ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x20;
        }
        else
        {
            Sfx_PlayFromObject(obj, SFXTRIG_baddie_mika_wingflap_260);
            ((EnemyState*)state)->current = 0;
            ((EnemyState*)state)->flags2E4 = ((EnemyState*)state)->flags2E4 | 0x20;
            ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 8;
        }
    }
    return;
}

void wbUpdateEngaged(u32 obj, int state)
{
    u32 randVal;
    GameObject* tracked;
    f32 moveSpeed;
    ObjHitsPriorityState* hitState;

    if (((EnemyState*)state)->duster.decoyTimer > 250.0f)
    {
        ((EnemyState*)state)->duster.decoyTimer = 150.0f;
    }
    hitState = (ObjHitsPriorityState*)((GameObject*)obj)->anim.hitReactState;
    hitState->suppressOutgoingHits = 0;
    ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, DUSTER_HIT_VOLUME_SLOT, 1, 0);
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        Sfx_PlayFromObject(obj, SFXTRIG_mn_heart1_c_261);
    }
    ((EnemyState*)state)->duster.decoyTimer = ((EnemyState*)state)->duster.decoyTimer - timeDelta;
    if (((EnemyState*)state)->duster.decoyTimer <= 0.0f)
    {
        if ((((EnemyState*)state)->controlFlags & 0x600) != 0)
        {
            randVal = randomGetRange(0x96, 0xfa);
            ((EnemyState*)state)->duster.decoyTimer = (float)(int)randVal;
        }
        else
        {
            randVal = randomGetRange(600, 0x352);
            ((EnemyState*)state)->duster.decoyTimer = (float)(int)randVal;
        }
        Sfx_PlayFromObject(obj, SFXTRIG_baddie_eba_pollenspin);
    }
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        ObjAnim_SetCurrentMove(obj, 3, 0.0f, ((EnemyState*)state)->rootMotionFlags);
    }
    if (((EnemyState*)state)->duster.phaseTimer > 0.0f)
    {
        ((EnemyState*)state)->duster.phaseTimer = ((EnemyState*)state)->duster.phaseTimer - timeDelta;
        if (((EnemyState*)state)->duster.phaseTimer <= 0.0f)
        {
            ((EnemyState*)state)->duster.phaseTimer = 600.0f;
            ((EnemyState*)state)->flags2E4 = ((EnemyState*)state)->flags2E4 | 0x10000LL;
        }
    }
    else if ((((EnemyState*)state)->controlFlags & 0x400) != 0)
    {
        ((EnemyState*)state)->duster.phaseTimer = 600.0f;
    }
    if ((((EnemyState*)state)->controlFlags & 0x8000000) != 0)
    {
        moveSpeed = 20.0f;
    }
    else
    {
        tracked = (GameObject*)((EnemyState*)state)->trackedObj;
        moveSpeed = sidekickToy_accelerateTowardTargetXZ(
            (GameObject*)(obj), tracked->anim.worldPosX, 50.0f + tracked->anim.worldPosY,
            tracked->anim.worldPosZ, 30.0f, 0.2f, 5.0f, ((EnemyState*)state)->drag);
    }
    if (((moveSpeed > 0.0f) && (((GameObject*)obj)->anim.velocityY < -0.5f)) ||
        ((((EnemyState*)state)->controlFlags & 0x8000000) != 0))
    {
        ((EnemyState*)state)->userData1 = 1;
    }
    if ((((EnemyState*)state)->userData1 != 0) && (moveSpeed > 0.0f))
    {
        ((EnemyState*)state)->animPlaySpeed = 0.05f;
        if (((EnemyState*)state)->current != 0)
        {
            ((GameObject*)obj)->anim.velocityY += 0.02f;
        }
        if (((GameObject*)obj)->anim.velocityY < -0.7f)
        {
            ((GameObject*)obj)->anim.velocityY = -0.7f;
        }
        else if (((GameObject*)obj)->anim.velocityY > 0.5f)
        {
            ((GameObject*)obj)->anim.velocityY = 0.5f;
        }
    }
    else
    {
        ((EnemyState*)state)->userData1 = 0;
        if (((EnemyState*)state)->animPlaySpeed > 0.025f)
        {
            ((EnemyState*)state)->animPlaySpeed = -(0.005f * timeDelta - ((EnemyState*)state)->animPlaySpeed);
        }
    }
    baddieTurnTowardLookDir((GameObject*)obj, (void*)state, 0x2d, 0.0f, 0.0f, 0);
}

void wbUpdateIdle(u32 obj, int state)
{
    u32 randVal;
    RomCurveWalker* route;
    ObjPlacement* placement;
    f32 moveSpeed;
    ObjHitsPriorityState* hitState;

    route = *(RomCurveWalker**)state;
    placement = ((GameObject*)obj)->anim.placement;
    hitState = (ObjHitsPriorityState*)((GameObject*)obj)->anim.hitReactState;
    hitState->suppressOutgoingHits = 0;
    ObjHits_SetHitVolumeSlot((ObjAnimComponent*)obj, DUSTER_HIT_VOLUME_SLOT, 1, 0);
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        Sfx_PlayFromObject(obj, SFXTRIG_mn_heart1_c_261);
    }
    ((EnemyState*)state)->duster.decoyTimer = ((EnemyState*)state)->duster.decoyTimer - timeDelta;
    if (((EnemyState*)state)->duster.decoyTimer <= 0.0f)
    {
        if ((((EnemyState*)state)->controlFlags & 0x600) != 0)
        {
            randVal = randomGetRange(0x96, 0xfa);
            ((EnemyState*)state)->duster.decoyTimer = (float)(int)randVal;
        }
        else
        {
            randVal = randomGetRange(600, 0x352);
            ((EnemyState*)state)->duster.decoyTimer = (float)(int)randVal;
        }
        Sfx_PlayFromObject(obj, SFXTRIG_baddie_eba_pollenspin);
    }
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        ObjAnim_SetCurrentMove(obj, 0, 0.0f, ((EnemyState*)state)->rootMotionFlags);
    }
    if (((EnemyState*)state)->duster.phaseTimer > 0.0f)
    {
        ((EnemyState*)state)->duster.phaseTimer = ((EnemyState*)state)->duster.phaseTimer - timeDelta;
        if (((EnemyState*)state)->duster.phaseTimer <= 0.0f)
        {
            ((EnemyState*)state)->duster.phaseTimer = 0.0f;
        }
    }
    else
    {
        ((EnemyState*)state)->flags2E4 = ((EnemyState*)state)->flags2E4 & ~0x10000LL;
    }
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_PATH_FOLLOW) != 0)
    {
        if (((Curve_AdvanceAlongPath(&route->curve, ((EnemyState*)state)->pathStep) != 0 ||
              route->atSegmentEnd != 0) &&
             (*gRomCurveInterface)->goNextPoint(route) != 0) &&
            (*gRomCurveInterface)
                    ->initCurve(*(RomCurveWalker**)state, (void*)obj, 700.0f, (int*)&gWbCurveInitData, -1) != 0)
        {
            ((EnemyState*)state)->controlFlags =
                ((EnemyState*)state)->controlFlags & ~(u64)BADDIE_CONTROL_PATH_FOLLOW;
        }
        if ((((EnemyState*)state)->controlFlags & 0x8000000) != 0)
        {
            moveSpeed = 30.0f;
        }
        else
        {
            moveSpeed = sidekickToy_accelerateTowardTargetXZ((GameObject*)(obj), route->posX, route->posY, route->posZ,
                                                             30.0f, 0.2f, 5.0f,
                                                             ((EnemyState*)state)->drag);
        }
    }
    else if ((((EnemyState*)state)->controlFlags & 0x8000000) != 0)
    {
        moveSpeed = 30.0f;
    }
    else
    {
        moveSpeed = sidekickToy_accelerateTowardTargetXZ((GameObject*)(obj), placement->posX, placement->posY,
                                                         placement->posZ, 30.0f, 0.2f, 5.0f,
                                                         ((EnemyState*)state)->drag);
    }
    if (((moveSpeed > 0.0f) && (((GameObject*)obj)->anim.velocityY < -0.5f)) ||
        ((((EnemyState*)state)->controlFlags & 0x8000000) != 0))
    {
        ((EnemyState*)state)->userData1 = 1;
    }
    if ((((EnemyState*)state)->userData1 != 0) && (moveSpeed > 0.0f))
    {
        ((EnemyState*)state)->animPlaySpeed = 0.05f;
        if (((EnemyState*)state)->current != 0)
        {
            ((GameObject*)obj)->anim.velocityY += 0.02f;
        }
        if (((GameObject*)obj)->anim.velocityY < -0.7f)
        {
            ((GameObject*)obj)->anim.velocityY = -0.7f;
        }
        else if (((GameObject*)obj)->anim.velocityY > 0.5f)
        {
            ((GameObject*)obj)->anim.velocityY = 0.5f;
        }
    }
    else
    {
        ((EnemyState*)state)->userData1 = 0;
        if (((EnemyState*)state)->animPlaySpeed > 0.025f)
        {
            ((EnemyState*)state)->animPlaySpeed = -(0.005f * timeDelta - ((EnemyState*)state)->animPlaySpeed);
        }
    }
    baddieTurnTowardLookDir((GameObject*)obj, (void*)state, 0x2d, 0.0f, 0.0f, 0);
}

void wbInit(u32 unused, int state)
{
    float fa;
    u32 ua;

    ((EnemyState*)state)->sightRange = 60.0f;
    ((EnemyState*)state)->flags2E4 = 0x2002b029;
    ((EnemyState*)state)->animPlaySpeed = 0.05f;
    ((EnemyState*)state)->gravity = 0.006f;
    ((EnemyState*)state)->drag = 0.95f;
    ((EnemyState*)state)->moveId0 = 0;
    fa = 1.0f;
    ((EnemyState*)state)->moveSpeedScale0 = 1.0f;
    ((EnemyState*)state)->moveId1 = 1;
    ((EnemyState*)state)->moveSpeedScale1 = fa;
    ((EnemyState*)state)->moveId2 = 2;
    ((EnemyState*)state)->moveSpeedScale2 = fa;
    ua = randomGetRange(0x78, 0x1e0);
    ((EnemyState*)state)->duster.decoyTimer = (float)(int)ua;
    return;
}

void mutatedEbaPlayMoveSfx(u32 obj, EnemyState* state);

void mutatedEbaPlayMoveSfx(u32 obj, EnemyState* state)
{
    switch (((GameObject*)obj)->anim.currentMove)
    {
    case 5:
        if (state->animEventMask != 0)
        {
            Sfx_PlayFromObject(obj, SFXTRIG_baddie_rach_bite);
        }
        break;
    case 6:
        if (state->animEventMask != 0)
        {
            Sfx_PlayFromObject(obj, SFXTRIG_baddie_rach_bite);
        }
        break;
    case 7:
        if (state->animEventMask != 0)
        {
            if (((GameObject*)obj)->anim.currentMoveProgress < 0.15f)
            {
                Sfx_PlayFromObject(obj, SFXTRIG_baddie_rach_bite);
            }
            else
            {
                Sfx_PlayFromObject(obj, SFXTRIG_baddie_kooshy_death);
            }
        }
        break;
    case 8:
        if (state->animEventMask != 0)
        {
            if (((GameObject*)obj)->anim.currentMoveProgress < 0.25f)
            {
                Sfx_PlayFromObject(obj, SFXTRIG_baddie_kooshy_hit);
            }
            else if (((GameObject*)obj)->anim.currentMoveProgress < 0.75f)
            {
                Sfx_PlayFromObject(obj, SFXTRIG_baddie_rach_call1);
            }
            else
            {
                Sfx_PlayFromObject(obj, SFXTRIG_baddie_kooshy_death);
            }
        }
        break;
    }
    return;
}

void mutatedEbaUpdateWhileFrozen(int obj, u8* state, int unused, int eventKind, int wpad0, int wpad1, Vec* wpad2,
                                 int wpad3)
{
    int move;

    if (eventKind != 0x11)
    {
        if (eventKind == 0x10)
        {
            ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x20;
        }
        else
        {
            if ((((move = ((GameObject*)obj)->anim.currentMove) == 0) || (move == 1)) || (move == 3) || (move == 4))
            {
                Sfx_PlayFromObject(obj, SFXTRIG_mv_ladderslide16_250);
                ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x10;
            }
            else
            {
                baddieSetMove((GameObject*)obj, (int)state, 4, 1.0f, 0, 0);
                ((EnemyState*)state)->userData1 = 0;
                Sfx_PlayFromObject(obj, SFXTRIG_baddie_kooshy_call);
                ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 8;
            }
        }
    }
    return;
}

void mutatedEbaUpdateEngaged(u32 obj, int state)
{
    int tblOff;

    ((ObjHitsPriorityState*)((GameObject*)obj)->anim.hitReactState)->hitVolumePriority = 10;
    ((ObjHitsPriorityState*)((GameObject*)obj)->anim.hitReactState)->hitVolumeId = 1;
    if (((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_JUST_TRIGGERED) != 0) &&
        (((EnemyState*)state)->userData1 <= 1))
    {
        ((EnemyState*)state)->userData1 = 1;
        ((EnemyState*)state)->controlFlags = ((EnemyState*)state)->controlFlags | (u64)BADDIE_CONTROL_SEQUENCE_DRIVEN;
    }
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        ((EnemyState*)state)->userData1 += 1;
        if (10 < ((EnemyState*)state)->userData1)
        {
            ((EnemyState*)state)->userData1 = 3;
        }
        if (((EnemyState*)state)->turnOctant < 4)
        {
            tblOff = (u32)((EnemyState*)state)->userData1 * 0xc;
            baddieSetMove((GameObject*)obj, state, gDusterEbaMoveTable[tblOff + 8],
                        *(float*)(gDusterEbaMoveTable + tblOff), 0, 0);
        }
        else
        {
            tblOff = (u32)((EnemyState*)state)->userData1 * 0xc;
            baddieSetMove((GameObject*)obj, state, gDusterEbaMoveTable[tblOff + 9],
                        *(float*)(gDusterEbaMoveTable + tblOff), 0, 0);
        }
    }
    mutatedEbaPlayMoveSfx(obj, (EnemyState*)state);
    return;
}

void mutatedEbaUpdateIdle(u32 obj, int state)
{
    int tblOff;
    u32 phase;

    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        phase = ((EnemyState*)state)->userData1;
        if (phase == 0)
        {
            ((EnemyState*)state)->userData1 += 1;
        }
        else if (phase >= 2)
        {
            ((EnemyState*)state)->userData1 = 0;
        }
        tblOff = (u32)((EnemyState*)state)->userData1 * 0xc;
        baddieSetMove((GameObject*)obj, state, gDusterEbaMoveTable[tblOff + 8],
                    *(float*)(gDusterEbaMoveTable + tblOff), 0, 0);
    }
    mutatedEbaPlayMoveSfx(obj, (EnemyState*)state);
    return;
}

void mutatedEbaInit(u32 unused, int state)
{
    float fa;

    ((EnemyState*)state)->sightRange = 60.0f;
    ((EnemyState*)state)->flags2E4 = 0x46001;
    ((EnemyState*)state)->animPlaySpeed = 0.01f;
    ((EnemyState*)state)->gravity = 0.006f;
    ((EnemyState*)state)->drag = 0.95f;
    ((EnemyState*)state)->moveId0 = 0;
    fa = 1.0f;
    ((EnemyState*)state)->moveSpeedScale0 = 1.0f;
    ((EnemyState*)state)->moveId1 = 4;
    ((EnemyState*)state)->moveSpeedScale1 = fa;
    ((EnemyState*)state)->moveId2 = 3;
    ((EnemyState*)state)->moveSpeedScale2 = fa;
    ((EnemyState*)state)->userData1 = 1;
    ((EnemyState*)state)->current = 0xa;
    return;
}

#define FIRECRAWLER_HIT_VOLUME_SLOT      9
/* group owned by another DLL, queried here */
#define LANTERNFIREFLY_OBJGROUP 0x30 /* DLL 0x10C lanternfirefly */

extern f32 gHoodedZyckFollowUpMoveSpeed;
extern f32 gHoodedZyckLungeMoveSpeed;
extern f32 gHoodedZyckLargeTargetSpeedScale;
extern f32 gHoodedZyckEmergeMoveSpeed;

void hoodedZyckUpdateWhileFrozen(int obj, u8* state, int unused, int eventKind, int wpad0, int wpad1, Vec* wpad2,
                                 int wpad3)
{
    if (eventKind == 0x10)
    {
        ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x20;
    }
    else
    {
        ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 8;
        Sfx_PlayFromObject(obj, SFXTRIG_dn_boar1_c_244);
        ((EnemyState*)state)->current = 0;
    }
    return;
}

static inline void hoodedZyck_tickPhaseTimer(EnemyState* st)
{
    st->duster.phaseTimer = st->duster.phaseTimer - timeDelta;
    if (st->duster.phaseTimer <= 0.0f)
    {
        st->duster.phaseTimer = (f32)(int)randomGetRange(0x3c, 0x78);
    }
}

static inline int hoodedZyck_getAngleDelta(GameObject* obj, GameObject* target)
{
    f32 d = (f32)(int)((u16)getAngle(obj->anim.localPosX - target->anim.localPosX,
                                     obj->anim.localPosZ - target->anim.localPosZ) -
                       (u16)obj->anim.rotX);
    if (d > 32768.0f)
    {
        d = -65535.0f + d;
    }
    if (d < -32768.0f)
    {
        d = 65535.0f + d;
    }
    return d;
}

void hoodedZyck_updateIdle(GameObject* obj, int state)
{
    bool resetting;
    int groundHit;
    u8 noHit;
    int randBit;
    float toPos[3];
    float fromPos[3];
    float cosYaw;
    float sinYaw;
    float hitOut[22];

    hoodedZyck_tickPhaseTimer((EnemyState*)state);
    if (0.0f != ((EnemyState*)state)->duster.decoyTimer)
    {
        ObjHits_DisableObject(obj);
        if ((obj)->anim.currentMove != 5)
        {
            baddieSetMove(obj, state, 5, gHoodedZyckEmergeMoveSpeed, 0, 0);
        }
        else if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            ObjHits_EnableObject(obj);
            ((EnemyState*)state)->duster.decoyTimer = 0.0f;
        }
        (obj)->anim.alpha = 0xff;
        resetting = true;
    }
    else
    {
        resetting = false;
    }
    if (!resetting)
    {
        (obj)->anim.rotX = (short)((obj)->anim.rotX + ((EnemyState*)state)->phaseAngle);
        fromPos[0] = (obj)->anim.localPosX;
        fromPos[1] = (obj)->anim.localPosY;
        fromPos[2] = (obj)->anim.localPosZ;
        angleToVec2Fast((u32)(u16)(obj)->anim.rotX, &sinYaw, &cosYaw);
        toPos[0] = (obj)->anim.localPosX - 10.0f * sinYaw;
        toPos[1] = 5.0f + (obj)->anim.localPosY;
        toPos[2] = (obj)->anim.localPosZ - 10.0f * cosYaw;
        groundHit = objBboxFn_800640cc(fromPos, toPos, 0.0f, 3, (TrackBBoxHit*)hitOut,
                                       obj,
                                       (u32) * (u8*)(state + 0x261),
                                       0xffffffff, 0xff, 0);
        noHit = !(groundHit & 0xff);
        if (!noHit || ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0))
        {
            if (noHit && (obj)->anim.currentMove != 0)
            {
                ((EnemyState*)state)->phaseAngle = 0;
                baddieSetMove(obj, state, 0, 1.0f, 0, 1);
            }
            else
            {
                float fz;
                baddieSetMove(obj, state, 1, 0.75f, 0, 0);
                fz = 0.0f;
                (obj)->anim.velocityX = fz;
                (obj)->anim.velocityY = fz;
                (obj)->anim.velocityZ = fz;
                randBit = randomGetRange(0, 1);
                ((EnemyState*)state)->phaseAngle = (u16)((randBit - 1) * 0x12c);
            }
        }
        (obj)->anim.rotY = ((EnemyState*)state)->spawnRotY;
        (obj)->anim.rotZ = ((EnemyState*)state)->spawnRotZ;
    }
    return;
}

void hoodedZyck_updateB(GameObject* obj, u8* state)
{
    f32 scale;
    int moved;
    int turnRaw;
    u8 noHit;
    u16 mag;
    u8 bufA[88];
    u8 bufB[84];
    f32 tgtA[3];
    f32 posA[3];
    f32 tgtB[3];
    f32 posB[3];
    f32 range;
    f32 cosA;
    f32 sinA;
    f32 cosB;
    f32 sinB;

    {
        u8 n = *(u8*)(obj->anim.placementDataAddress + 0x2f);
        scale = n;
        if (0.0f == n)
        {
            scale = 10.0f;
        }
        scale = scale / 10.0f;
    }

    hoodedZyck_tickPhaseTimer((EnemyState*)state);

    if (0.0f != ((EnemyState*)state)->crawler.emergeTimer)
    {
        ObjHits_DisableObject(obj);
        if (obj->anim.currentMove != 5)
        {
            baddieSetMove(obj, (int)state, 5, gHoodedZyckEmergeMoveSpeed, 0, 0);
        }
        else if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            ObjHits_EnableObject(obj);
            ((EnemyState*)state)->crawler.emergeTimer = 0.0f;
        }
        obj->anim.alpha = 0xff;
        moved = 1;
    }
    else
    {
        moved = 0;
    }

    if (moved == 0)
    {
        u32 ang;
        f32 diff;
        GameObject* other;

        *(s16*)obj = *(s16*)obj + ((EnemyState*)state)->phaseAngle;
        posA[0] = obj->anim.localPosX;
        posA[1] = obj->anim.localPosY;
        posA[2] = obj->anim.localPosZ;
        angleToVec2Fast((u16)obj->anim.rotX, &sinA, &cosA);
        tgtA[0] = -(10.0f * sinA - obj->anim.localPosX);
        tgtA[1] = 5.0f + obj->anim.localPosY;
        tgtA[2] = -(10.0f * cosA - obj->anim.localPosZ);
        noHit = !(u8)objBboxFn_800640cc(posA, tgtA, 0.0f, 3, (TrackBBoxHit*)bufA, obj,
                                        ((EnemyState*)state)->unk261, -1, 0xff, 0);
        ang =
            getAngle(
                obj->anim.localPosX - ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX,
                obj->anim.localPosZ - ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ) &
            0xffff;
        diff = (f32)(int)(ang - ((int)*(s16*)obj & 0xffffu));
        if (diff > 32768.0f)
        {
            diff = -65535.0f + diff;
        }
        if (diff < -32768.0f)
        {
            diff = 65535.0f + diff;
        }
        turnRaw = diff;
        {
            s16 t = turnRaw;
            mag = (u16)(t >= 0 ? t : -t);
        }
        if (playerFindNearestFirefly(Obj_GetPlayerObject()) != 0)
        {
            range = 100.0f;
            other = (GameObject*)ObjGroup_FindNearestObject(LANTERNFIREFLY_OBJGROUP, obj, &range);
            if (other != NULL)
            {
                s16 yaw = Obj_GetYawDeltaToObject(obj, other, &range);
                int t;
                if (yaw < -300)
                {
                    yaw = -300;
                }
                else if (yaw > 300)
                {
                    yaw = 300;
                }
                t = yaw;
                ((EnemyState*)state)->phaseAngle = t;
                t = yaw >= 0 ? yaw : -yaw;
                if (t < 0x4000)
                {
                    *(s16*)obj = -*(s16*)obj;
                    posB[0] = obj->anim.localPosX;
                    posB[1] = obj->anim.localPosY;
                    posB[2] = obj->anim.localPosZ;
                    angleToVec2Fast((u16)obj->anim.rotX, &sinB, &cosB);
                    tgtB[0] = -(10.0f * sinB - obj->anim.localPosX);
                    tgtB[1] = 5.0f + obj->anim.localPosY;
                    tgtB[2] = -(10.0f * cosB - obj->anim.localPosZ);
                    if ((u8)objBboxFn_800640cc(posB, tgtB, 0.0f, 3, (TrackBBoxHit*)bufB, obj,
                                               *(u8*)(state + 0x261), -1, 0xff, 0) == 0)
                    {
                        if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
                        {
                            baddieSetMove(obj, (int)state, 7, 1.0f / (2.0f * scale), 0, 1);
                        }
                        obj->anim.rotY = ((EnemyState*)state)->spawnRotY;
                        obj->anim.rotZ = ((EnemyState*)state)->spawnRotZ;
                    }
                    *(s16*)obj = -*(s16*)obj;
                }
                return;
            }
        }
        if (((EnemyState*)state)->trackedObj != NULL &&
            ((GameObject*)((EnemyState*)state)->trackedObj)->anim.hitboxScale > 56.0f)
        {
            ((EnemyState*)state)->sightRange = gHoodedZyckLargeTargetSpeedScale;
        }
        if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0 || noHit == 0 ||
            (mag < 3000 && noHit != 0 && obj->anim.currentMove != 0))
        {
            if (noHit != 0 && mag < 3000)
            {
                ((EnemyState*)state)->phaseAngle = 0;
                baddieSetMove(obj, (int)state, 0, 1.0f / scale, 0, 1);
            }
            else
            {
                baddieSetMove(obj, (int)state, 1, 0.75f / scale, 0, 0);
                {
                    f32 z = 0.0f;
                    obj->anim.velocityX = z;
                    obj->anim.velocityY = z;
                    obj->anim.velocityZ = z;
                }
                if (mag < 3000)
                {
                    ((EnemyState*)state)->phaseAngle = (randomGetRange(0, 1) - 1) * 300;
                }
                else if ((s16)turnRaw < 0)
                {
                    ((EnemyState*)state)->phaseAngle = 0xfed4;
                }
                else
                {
                    ((EnemyState*)state)->phaseAngle = 300;
                }
            }
        }
        obj->anim.rotY = ((EnemyState*)state)->spawnRotY;
        obj->anim.rotZ = ((EnemyState*)state)->spawnRotZ;
    }
}

void hoodedZyck_update(GameObject* obj, u8* state)
{
    int moved;
    int turnRaw;
    u16 mag;
    u32 grabbed;

    hoodedZyck_tickPhaseTimer((EnemyState*)state);

    if (0.0f != ((EnemyState*)state)->crawler.emergeTimer)
    {
        ObjHits_DisableObject(obj);
        if (obj->anim.currentMove != 5)
        {
            baddieSetMove(obj, (int)state, 5, gHoodedZyckEmergeMoveSpeed, 0, 0);
        }
        else if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            ObjHits_EnableObject(obj);
            ((EnemyState*)state)->crawler.emergeTimer = 0.0f;
        }
        obj->anim.alpha = 0xff;
        moved = 1;
    }
    else
    {
        moved = 0;
    }

    if (moved == 0)
    {
        f32 z;
        *(s16*)obj = (f32)((EnemyState*)state)->phaseAngle * timeDelta + (f32)(int)*(s16*)obj;
        z = 0.0f;
        obj->anim.velocityX = z;
        obj->anim.velocityY = z;
        obj->anim.velocityZ = z;
        ObjHits_SetHitVolumeSlot(&obj->anim, FIRECRAWLER_HIT_VOLUME_SLOT, 1, -1);
        turnRaw = hoodedZyck_getAngleDelta(obj, (GameObject*)((EnemyState*)state)->trackedObj);
        {
            int t = (s16)turnRaw;
            mag = (u16)(t >= 0 ? t : -t);
        }
        ObjHits_EnableObject(obj);
        grabbed = ((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN;
        if (grabbed != 0 && obj->anim.currentMove == 6)
        {
            baddieSetMove(obj, (int)state, 4, gHoodedZyckFollowUpMoveSpeed, 0, 1);
        }
        else
        {
            if (grabbed != 0 ||
                (mag < 1000 && obj->anim.currentMove != 2 && obj->anim.currentMove != 4 &&
                 obj->anim.currentMove != 6))
            {
                if (mag < 1000)
                {
                    if (((EnemyState*)state)->sightRange < 40.0f)
                    {
                        baddieSetMove(obj, (int)state, 2, 0.75f, 0, 0);
                    }
                    else
                    {
                        baddieSetMove(obj, (int)state, 6, gHoodedZyckLungeMoveSpeed, 0, 0);
                    }
                    ((EnemyState*)state)->phaseAngle = 0;
                }
                else
                {
                    baddieSetMove(obj, (int)state, 1, 0.75f, 0, 0);
                    if ((s16)turnRaw < 0)
                    {
                        ((EnemyState*)state)->phaseAngle = 0xfed4;
                    }
                    else
                    {
                        ((EnemyState*)state)->phaseAngle = 300;
                    }
                }
            }
            obj->anim.rotY = ((EnemyState*)state)->spawnRotY;
            obj->anim.rotZ = ((EnemyState*)state)->spawnRotZ;
        }
    }
}

void hoodedZyck_init(GameObject* obj, EnemyState* st)
{
    f32 ratio;
    f32 base_v;
    u32 flags;
    u32 amt;
    amt = *((u8*)((int*)obj->anim.placementDataAddress) + 0x2f);
    ratio = amt;
    if (0.0f == amt)
    {
        ratio = 10.0f;
    }
    ratio = ratio / 10.0f;
    st->sightRange = 30.0f;
    st->flags2E4 = 0x8b;
    flags = st->flags2E4;
    st->flags2E4 = flags | 0x20;
    st->animPlaySpeed = 0.02f * ratio;
    base_v = 1.0f;
    st->gravity = base_v;
    st->drag = 0.97f;
    *((u8*)st + 0x320) = 0;
    st->moveSpeedScale0 = 1.5f;
    *((u8*)st + 0x321) = 3;
    {
        f32 d2 = 2.0f;
        st->moveSpeedScale1 = d2;
        *((u8*)st + 0x322) = 5;
        st->moveSpeedScale2 = d2;
    }
    ((EnemyState*)st)->phaseAngle = 0;
    ((EnemyState*)st)->crawler.engineTimer = 60.0f;
    ((EnemyState*)st)->crawler.emergeTimer = base_v;
    obj->anim.alpha = 0;
    st->pathStep = 0.5f * ratio;
    st->flags2E8 = 0;
    ObjHits_EnableObject(obj);
}

f32 gHoodedZyckFollowUpMoveSpeed = 0.7f;
f32 gHoodedZyckLungeMoveSpeed = 2.0f;
f32 gHoodedZyckLargeTargetSpeedScale = 110.0f;
f32 gHoodedZyckEmergeMoveSpeed = 2.0f;
int gCrawlerCurveInitData[2] = {2, 3};
int gHagabonMK2CurveInitData[2] = {2, 3};
extern u8 gHagabonMK2ModelChain0BoneIds[];
extern u8 gHagabonMK2ModelChain1BoneIds[];
extern u8 gHagabonMK2ModelChain2BoneIds[];
extern u8 gHagabonMK2ModelChain3BoneIds[];
extern u8 gHagabonMK2ModelChain4BoneIds[];

typedef struct CrawlerModelChainList
{
    u8* modelIds;
    s32 count;
} CrawlerModelChainList;

STATIC_ASSERT(sizeof(CrawlerModelChainList) == 8);

CrawlerModelChainList gHagabonMK2ModelChain0 = {gHagabonMK2ModelChain0BoneIds, 6};
CrawlerModelChainList gHagabonMK2ModelChain1 = {gHagabonMK2ModelChain1BoneIds, 6};
CrawlerModelChainList gHagabonMK2ModelChain2 = {gHagabonMK2ModelChain2BoneIds, 6};
CrawlerModelChainList gHagabonMK2ModelChain3 = {gHagabonMK2ModelChain3BoneIds, 6};
CrawlerModelChainList gHagabonMK2ModelChain4 = {gHagabonMK2ModelChain4BoneIds, 5};
u8 gSnowwormSeqIndexReset[4] = {2, 2, 0, 0};
u8 gSnowwormSeqIndexMax[4] = {0xD, 7, 0, 0};
u8 gSnowwormTurnRates[4] = {0x3C, 0xB4, 0, 0};
u8 gSnowwormHitReactionSeqIndices[4] = {3, 5, 9, 0xB};
u8 gSnowwormBabyHitReactionSeqIndices[8] = {3, 5, 3, 5, 0, 0, 0, 0};

#define FIRECRAWLER_OBJFLAG_RENDERED     0x800
#define FIRECRAWLER_OBJFLAG_PARENT_SLACK 0x1000
#define FIREHOLE_OBJ_ID                  0x710 /* FireHole child spawned by firecrawler (firepipe DLL 0x273) */
#define FIRECRAWLER_PROJECTILE_OBJ       0x869 /* retail "FireCrawler..." (DLL 0xD7 kaldachompspit) */
/* crawler-family enemy anim.romDefNos (docblock table: romDefNo -> enemy name) */
#define FIRECRAWLER_SEQID_FIRECRAWLER  0x6a2 /* FireCrawler */
#define FIRECRAWLER_SEQID_REDEYE       0x6a3 /* RedEye */
/* attacker romDefNo this creature is immune to (retail OBJECTS.bin). */
#define FIRECRAWLER_ATTACKER_SEQID_FLAMETHROWER 0x1b5 /* "FlameThrowe" (DLL 0xE4) */
#define FIRECRAWLER_SEQID_SHADOWHUNTER 0x6a4 /* ShadowHunter */

/* movement dust spawned on the move-loop event: turning (turnDelta != 0) */
#define FIRECRAWLER_PARTFX_MOVE_TURN 0x802
/* movement dust spawned on the move-loop event: moving straight (turnDelta == 0) */
#define FIRECRAWLER_PARTFX_MOVE_STRAIGHT 0x809

extern void* gCrawlerDescriptorTable[];

EnemyTargetSearchResult gCrawlerNearbyObjectBuffer[16];

f32 gCrawlerHitSfxTimer;


extern u8 gRedEyeLocomotionMoves[];
extern u8 gRedEyeRandomMoves[];
extern u8 gRedEyeDefaultMoveChain[];
extern u8 gRedEyeMoveChain[];
extern u8 gRedEyeOctantMoves[];
extern u8 gRedEyeMoveHitVolumes[];
extern u8 gRedEyeHitReactionSeq[];
extern u8 gCrawlerDefaultMoveEventFx[];
extern u8 gFireCrawlerLocomotionMoves[];
extern u8 gFireCrawlerOctantMoves[];
extern u8 gFireCrawlerRandomMoves[];
extern u8 gFireCrawlerMoveChain[];
extern u8 gFireCrawlerMoveHitVolumes[];
extern u8 gFireCrawlerHitReactionSeq[];
extern u8 gFireCrawlerMoveEventFx[];
extern u8 gShadowHunterLocomotionMoves[];
extern u8 gShadowHunterOctantMoves[];
extern u8 gShadowHunterRandomMoves[];
extern u8 gShadowHunterMoveChain[];
extern u8 gShadowHunterDefaultMoveChain[];
extern u8 gShadowHunterMoveHitVolumes[];
extern u8 gShadowHunterHitReactionSeq[];
void* gCrawlerDescriptorTable[24] = {
    gRedEyeLocomotionMoves, gRedEyeMoveChain, gRedEyeRandomMoves, gRedEyeOctantMoves, gRedEyeDefaultMoveChain, gRedEyeHitReactionSeq, gRedEyeMoveHitVolumes, gCrawlerDefaultMoveEventFx,
    gFireCrawlerLocomotionMoves, gFireCrawlerMoveChain, gFireCrawlerRandomMoves, gFireCrawlerOctantMoves, gFireCrawlerMoveChain, gFireCrawlerHitReactionSeq, gFireCrawlerMoveHitVolumes, gFireCrawlerMoveEventFx,
    gShadowHunterLocomotionMoves, gShadowHunterMoveChain, gShadowHunterRandomMoves, gShadowHunterOctantMoves, gShadowHunterDefaultMoveChain, gShadowHunterHitReactionSeq, gShadowHunterMoveHitVolumes, gCrawlerDefaultMoveEventFx,
};
typedef struct
{
    f32 speeds[3][3];
    f32 unused;
} CrawlerSpeedThresholdTable;
CrawlerSpeedThresholdTable gCrawlerSpeedThresholds = {
    {{1.2f, 0.8f, 1e-04f}, {1.2f, 0.8f, 1e-04f}, {1.2f, 0.8f, 1e-04f}},
    0.0f,
};
u8 gCrawlerSeqTable[] = {
    0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x02, 0x02, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x04, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x02, 0x04, 0x05, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
};
u8 gHagabonMK2ModelChain0BoneIds[0x18] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03,
                         0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x06};
u8 gHagabonMK2ModelChain1BoneIds[0x18] = {0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x0e,
                         0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x11};
u8 gHagabonMK2ModelChain2BoneIds[0x18] = {0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x14,
                         0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x17};
u8 gHagabonMK2ModelChain3BoneIds[0x18] = {0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x1a,
                         0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x1d};
u8 gHagabonMK2ModelChain4BoneIds[0x14] = {0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00,
                         0x00, 0x09, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x0b};
void* gCrawlerModelChainIds[] = {
    &gHagabonMK2ModelChain0, &gHagabonMK2ModelChain1, &gHagabonMK2ModelChain2, &gHagabonMK2ModelChain3, &gHagabonMK2ModelChain4,
};
u8 gSnowwormMoveSequence[0xa8] = {
    0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x07, 0x07, 0x07, 0x00, 0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0xcc, 0xcc, 0xcd, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x3e, 0xcc,
    0xcc, 0xcd, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0x02, 0x00, 0x3f, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x03, 0x03,
    0x03, 0x09, 0x00, 0x3f, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x03, 0x08, 0x08, 0x08, 0x00, 0x3f, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x07, 0x07, 0x07, 0x00, 0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x3e, 0xcc, 0xcc, 0xcd, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x3e, 0xcc, 0xcc, 0xcd, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x02, 0x02, 0x00, 0x3f, 0xa6, 0x66, 0x66, 0x00, 0x00, 0x00, 0x03, 0x04, 0x04, 0x09, 0x00, 0x3f,
    0x33, 0x33, 0x33, 0x00, 0x00, 0x00, 0x03, 0x08, 0x08, 0x08, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x07, 0x07, 0x07, 0x00, 0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
u8 gSnowwormBabyMoveSequence[0x60] = {0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x07, 0x07, 0x07, 0x00, 0x40, 0x20,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0xcc, 0xcc, 0xcd,
                         0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x3f, 0x19, 0x99, 0x9a, 0x00, 0x00,
                         0x00, 0x01, 0x02, 0x02, 0x02, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
                         0x09, 0x09, 0x09, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x08, 0x08,
                         0x08, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x07, 0x07, 0x07, 0x00,
                         0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
u8* gCrawlerReactionTables[] = {
    gSnowwormMoveSequence,
    gSnowwormHitReactionSeqIndices,
    gSnowwormBabyMoveSequence,
    gSnowwormBabyHitReactionSeqIndices,
};
typedef struct
{
    f32 spd;   /* 0x0 */
    u32 mask;  /* 0x4 */
    u8 moveId; /* 0x8 */
    u8 next;   /* 0x9 */
    u8 mode;   /* 0xa */
    u8 pad;
} CrawlerSeq12;
typedef struct
{
    f32 spd;   /* 0x0 */
    u32 mask;  /* 0x4 */
    u8 moveId; /* 0x8 */
    u8 next9;  /* 0x9 */
    u8 nextA;  /* 0xa */
    u8 pad;
    int flagC; /* 0xc */
} CrawlerSeq16;
typedef struct
{
    u8 pad[6];
    u16 sfxId; /* 0x6 */
    f32 vol;   /* 0x8 */
    f32 x;     /* 0xc */
    f32 y;     /* 0x10 */
    f32 z;     /* 0x14 */
} CrawlerSfxParams;
typedef struct
{
    u8* tbl0;          // 0x0  anim move ids
    CrawlerSeq12* tbl4;  // 0x4  chained move table
    CrawlerSeq12* tbl8;  // 0x8  random move table
    CrawlerSeq12* tblC;  // 0xc  octant move table
    CrawlerSeq12* tbl10; // 0x10 single move entry
    CrawlerSeq16* seq;   // 0x14
    CrawlerSeq12* tbl18; // 0x18 anim-id loop table
    u8 pad1C[4];
} CrawlerDescriptor;

/* Nearby-object scan. Asks enemy_findNearbyEnemies for up to 40 objects
 * within 640.0f, walks the result array of (obj, ?) pairs, and if
 * any entry's modelType is 0x6a3 with state[0x2dc] bit 0x20000000 set
 * AND bits 0x1800 clear, latches "found" and exits. If nothing matched,
 * loads the default triggered camera action. */
void crawler_checkNearbyActive(GameObject* obj, u8* state)
{
    u8 count = enemy_findNearbyEnemies(obj, 640.0f, 0, 0x28, gCrawlerNearbyObjectBuffer);
    u8 noMatch = 1;
    if (count >= 1)
    {
        u8 i;
        for (i = 0; i < count; i++)
        {
            u32 objectIndex = (u8)i;
            GameObject* e = gCrawlerNearbyObjectBuffer[objectIndex].obj;
            if (e->anim.romDefNo == FIRECRAWLER_SEQID_REDEYE)
            {
                u32 flags = ((EnemyState*)e->extra)->controlFlags;
                if ((flags & 0x20000000) != 0 && (flags & 0x1800) == 0)
                {
                    i = count;
                    noMatch = 0;
                }
            }
        }
    }
    if (noMatch != 0)
    {
        (*gCameraInterface)->loadTriggeredCamAction(0, 0, 0);
    }
}

void firecrawler_spawnFireHole(GameObject* obj, u8* state)
{
    FirePipeMapData* setup;
    GameObject* child;
    (void)state;
    if (Obj_IsLoadingLocked() != 0)
    {
        setup = (FirePipeMapData*)Obj_AllocObjectSetup(0x24, FIREHOLE_OBJ_ID);
        ObjPath_GetPointWorldPosition(obj, 0, &setup->base.posX, &setup->base.posY, &setup->base.posZ, 0);
        setup->base.color[0] = 1;
        setup->base.color[1] = 4;
        setup->base.color[2] = 0xff;
        setup->base.color[3] = 0xff;
        setup->rotX = 0;
        setup->rotY = 0;
        setup->cycleTime = 0;
        setup->scale = 0xa;
        setup->gameBit = 0;
        setup->startOffset = 0;
        setup->flags = 3;
        setup->pad23 = 0;
        child = Obj_SetupObject(&setup->base, 5, -1, -1, 0);
        if (child != NULL)
        {
            ObjLink_AttachChild(obj, child, 0);
            firepipe_setLinkedUpdateFlag(child);
            child->anim.flags = (s16)(child->anim.flags | OBJANIM_FLAG_HIDDEN);
        }
    }
}

void firecrawler_spawnProjectile(GameObject* obj, u8* state)
{
    u8 locked = Obj_IsLoadingLocked();
    if (locked != 0)
    {
        GameObject* child;
        int setup = (int)Obj_AllocObjectSetup(0x24, FIRECRAWLER_PROJECTILE_OBJ);
        ObjPath_GetPointWorldPosition(obj, 0, (f32*)(setup + 8), (f32*)(setup + 0xc), (f32*)(setup + 0x10),
                                      0);
        ((ObjPlacement*)setup)->color[0] = 1;
        ((ObjPlacement*)setup)->color[1] = 4;
        ((ObjPlacement*)setup)->color[2] = 0xff;
        ((ObjPlacement*)setup)->color[3] = 0xff;
        child = (GameObject*)((int)Obj_SetupObject((ObjPlacement*)setup, 5, -1, -1, 0));
        if ((u32)child != 0)
        {
            f32 dur = 60.0f * ((f32)((EnemyState*)state)->targetDist / ((EnemyState*)state)->aggroRange);
            child->anim.velocityX = (((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX -
                                                    ((ObjPlacement*)setup)->posX) /
                                                   dur;
            child->anim.velocityY =
                ((30.0f + ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosY +
                  (f32)(int)randomGetRange(-10, 10)) -
                 ((ObjPlacement*)setup)->posY) /
                dur;
            child->anim.velocityZ = (((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ -
                                                    ((ObjPlacement*)setup)->posZ) /
                                                   dur;
        }
        Sfx_PlayFromObject((int)obj, SFXTRIG_en_cvdrip1c_4ae);
    }
}

void crawlerPlayMoveEventFx(GameObject* obj, u8* state)
{
    typedef struct
    {
        u8 pad[4];
        u32 sfxId; /* 0x4 */
        u8 pad2;
        u8 shakeAmt;  /* 0x9 */
        u8 rumbleAmt; /* 0xa */
        u8 flags;     /* 0xb */
    } CrawlerSubDesc;
    typedef struct
    {
        u8 pad[0x1c];
        CrawlerSubDesc* p;
    } CrawlerDescE;
    CrawlerDescE* d = (CrawlerDescE*)gCrawlerDescriptorTable;
    CrawlerSubDesc* sub;
    CrawlerSubDesc* entry = d[((EnemyState*)state)->userData2].p;
    u8 i;

    gCrawlerHitSfxTimer = gCrawlerHitSfxTimer - timeDelta;

    for (i = 0; i <= 12; i++)
    {
        if ((((EnemyState*)state)->animEventMask & (1 << i)) != 0)
        {
            sub = &entry[i];
            if (sub->sfxId != 0)
            {
                Sfx_PlayFromObject((int)obj, sub->sfxId);
            }
            if (sub->shakeAmt != 0)
            {
                CameraShake_ApplyRadial(obj->anim.localPosX, obj->anim.localPosY,
                                        obj->anim.localPosZ, 160.0f, (f32)(u32)sub->shakeAmt);
            }
            if (sub->rumbleAmt != 0)
            {
                GameObject* player = Obj_GetPlayerObject();
                if ((player->objectFlags & FIRECRAWLER_OBJFLAG_PARENT_SLACK) == 0)
                {
                    f32 dist =
                        Vec_distance(&obj->anim.worldPosX, &player->anim.worldPosX);
                    if (dist <= 640.0f)
                    {
                        f32 amt = 1.0f - dist / 640.0f;
                        doRumble(amt * (f32)(u32)sub->rumbleAmt);
                    }
                }
            }
            if (sub->flags != 0)
            {
                if ((sub->flags & 1) != 0)
                {
                    ((EnemyState*)state)->familyData.crawler.flagsD = (u8)(((EnemyState*)state)->familyData.crawler.flagsD ^ 0x40);
                    if ((((EnemyState*)state)->familyData.crawler.flagsD & 0x40) != 0)
                    {
                        if (obj->childObjs[0] == NULL)
                        {
                            firecrawler_spawnFireHole(obj, state);
                        }
                        else
                        {
                            firepipe_setLinkedUpdateFlag(obj->childObjs[0]);
                        }
                    }
                    else if (obj->childObjs[0] != NULL)
                    {
                        firepipe_clearLinkedUpdateFlag(obj->childObjs[0]);
                    }
                }
                if ((sub->flags & 2) != 0)
                {
                    firecrawler_spawnProjectile(obj, state);
                }
            }
        }
    }
}

void crawler_onHit(GameObject* obj, u8* state, GameObject* attacker, int cmd, int p5, int damage, Vec* wpad0, int wpad1)
{
    typedef struct
    {
        u8 pad[0x14];
        CrawlerSeq16* seq; // 0x14
        u8 pad2[8];
    } CrawlerDesc;
    u8 idx;
    CrawlerDesc* d = (CrawlerDesc*)gCrawlerDescriptorTable;
    CrawlerSeq16* tbl = d[(idx = ((EnemyState*)state)->userData2)].seq;

    if (cmd == 0xe)
    {
        damage = damage << 3;
    }
    if (idx == 0 && cmd == 5)
    {
        damage = damage << 2;
    }
    if (idx == 1 &&
        (attacker->anim.romDefNo == FIRECRAWLER_ATTACKER_SEQID_FLAMETHROWER || attacker->anim.classId == 0x1c || cmd == 0x1f))
    {
        return;
    }
    if ((((EnemyState*)state)->familyData.crawler.flagsC & 4) != 0 || (idx == 0 && (((EnemyState*)state)->flags2F1 & 0x40) != 0))
    {
        if (cmd == 0x11)
        {
            return;
        }
        if ((obj)->anim.romDefNo == FIRECRAWLER_SEQID_FIRECRAWLER)
        {
            if (gCrawlerHitSfxTimer <= 0.0f && attacker != NULL)
            {
                switch (attacker->anim.romDefNo)
                {
                case 0x416:
                    Sfx_PlayFromObject((int)obj, SFXTRIG_snort);
                    break;
                case 0:
                case 0x69:
                    Sfx_PlayFromObject((int)obj, SFXTRIG_stftest);
                    break;
                }
                gCrawlerHitSfxTimer = 100.0f;
            }
        }
        else
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_swd_var);
        }
        ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x10;
        return;
    }

    if (idx == 1 && (obj)->childObjs[0] != NULL)
    {
        firepipe_clearLinkedUpdateFlag((obj)->childObjs[0]);
    }
    ((EnemyState*)state)->familyData.crawler.flagsD = ((EnemyState*)state)->familyData.crawler.flagsD & ~0x40;
    ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 & ~0x40LL;
    if (cmd == 0x10 && ((EnemyState*)state)->userData2 != 0)
    {
        ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x20;
        return;
    }

    if (((EnemyState*)state)->familyData.crawler.reactStep != 0)
    {
        u8 step;
        if (((EnemyState*)state)->userData2 == 0)
        {
            step = 4;
        }
        else
        {
            step = 3;
        }
        baddieSetMove(obj, (int)state, tbl[step].moveId, tbl[step].spd, 0, tbl[step].mask & 0xff);
        ((EnemyState*)state)->familyData.crawler.flagsC = tbl[step].flagC;
        (obj)->hitVolumeIndex = ((EnemyState*)state)->familyData.crawler.flagsC & 1;
        ((EnemyState*)state)->familyData.crawler.reactStep = tbl[step].next9;
        ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 8;
        if ((obj)->anim.romDefNo == FIRECRAWLER_SEQID_FIRECRAWLER)
        {
            if (gCrawlerHitSfxTimer <= 0.0f && attacker != NULL)
            {
                switch (attacker->anim.romDefNo)
                {
                case 0x416:
                    Sfx_PlayFromObject((int)obj, SFXTRIG_snort);
                    break;
                case 0:
                case 0x69:
                    Sfx_PlayFromObject((int)obj, SFXTRIG_stftest);
                    break;
                }
                Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_var);
                gCrawlerHitSfxTimer = 100.0f;
            }
        }
        else
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_stftest_var);
        }
        if (damage > ((EnemyState*)state)->current)
        {
            ((EnemyState*)state)->current = 0;
        }
        else
        {
            ((EnemyState*)state)->current = ((EnemyState*)state)->current - damage;
        }
        if (((EnemyState*)state)->current == 0 && ((EnemyState*)state)->userData2 == 0)
        {
            crawler_checkNearbyActive(obj, state);
        }
        return;
    }

    if ((((EnemyState*)state)->userData2 == 0 && cmd == 0x11 &&
         mainGetBit(GAMEBIT_STAFF_ABILITY_SUPER_QUAKE) != 0) ||
        ((EnemyState*)state)->userData2 == 1)
    {
        u8 v;
        baddieSetMove(obj, (int)state, tbl[1].moveId, tbl[1].spd, 0, tbl[1].mask & 0xff);
        ((EnemyState*)state)->familyData.crawler.flagsC = tbl[1].flagC;
        (obj)->hitVolumeIndex = ((EnemyState*)state)->familyData.crawler.flagsC & 1;
        ((EnemyState*)state)->familyData.crawler.reactStep = tbl[1].next9;
        v = ((EnemyState*)state)->userData2;
        if (v == 0)
        {
            ((EnemyState*)state)->crawler.emergeTimer = 6.0f * (f32)((EnemyState*)state)->hitStunFrames;
            ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 8;
            if ((obj)->anim.romDefNo == FIRECRAWLER_SEQID_FIRECRAWLER)
            {
                if (gCrawlerHitSfxTimer <= 0.0f && attacker != NULL)
                {
                    switch (attacker->anim.romDefNo)
                    {
                    case 0x416:
                        Sfx_PlayFromObject((int)obj, SFXTRIG_snort);
                        break;
                    case 0:
                    case 0x69:
                        Sfx_PlayFromObject((int)obj, SFXTRIG_stftest);
                        break;
                    }
                    Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_var);
                    gCrawlerHitSfxTimer = 100.0f;
                }
            }
            else
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_stftest_var);
            }
            return;
        }
        if (v == 1)
        {
            ((EnemyState*)state)->crawler.emergeTimer = 2.0f * (f32)((EnemyState*)state)->hitStunFrames;
            if ((obj)->anim.romDefNo == FIRECRAWLER_SEQID_FIRECRAWLER)
            {
                if (gCrawlerHitSfxTimer <= 0.0f && attacker != NULL)
                {
                    switch (attacker->anim.romDefNo)
                    {
                    case 0x416:
                        Sfx_PlayFromObject((int)obj, SFXTRIG_snort);
                        break;
                    case 0:
                    case 0x69:
                        Sfx_PlayFromObject((int)obj, SFXTRIG_stftest);
                        break;
                    }
                    Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_var);
                    gCrawlerHitSfxTimer = 100.0f;
                }
            }
            else
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_swd_var);
            }
            ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x10;
        }
        return;
    }

    if (cmd != 0x11)
    {
        if ((obj)->anim.romDefNo == FIRECRAWLER_SEQID_FIRECRAWLER)
        {
            if (gCrawlerHitSfxTimer <= 0.0f && attacker != NULL)
            {
                switch (attacker->anim.romDefNo)
                {
                case 0x416:
                    Sfx_PlayFromObject((int)obj, SFXTRIG_snort);
                    break;
                case 0:
                case 0x69:
                    Sfx_PlayFromObject((int)obj, SFXTRIG_stftest);
                    break;
                }
                Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_var);
                gCrawlerHitSfxTimer = 100.0f;
            }
        }
        else
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_swd_var);
        }
    }
    ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x10;
}

void crawler_updateC(GameObject* obj, u8* state)
{
    CrawlerDescriptor* d = (CrawlerDescriptor*)gCrawlerDescriptorTable;
    CrawlerSeq12* t8 = d[((EnemyState*)state)->userData2].tbl8;
    u8* t0 = d[((EnemyState*)state)->userData2].tbl0;
    CrawlerSeq16* seq = d[((EnemyState*)state)->userData2].seq;
    CrawlerSeq12* tC = d[((EnemyState*)state)->userData2].tblC;
    RomCurveWalker* base = *(RomCurveWalker**)state;
    f32 scale = 1.0f;
    f32 cap;
    int i;
    f32 dv[3];

    ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 & ~0x40LL;
    if (obj->childObjs[0] != NULL)
    {
        firepipe_clearLinkedUpdateFlag(obj->childObjs[0]);
    }

    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_JUST_TRIGGERED) != 0)
    {
        ((EnemyState*)state)->familyData.crawler.flagsD = ((EnemyState*)state)->familyData.crawler.flagsD | 8;
        if ((*gRomCurveInterface)->initCurve(*(RomCurveWalker**)state, obj, 0.0f, (int*)&gCrawlerCurveInitData, -1) != 0)
        {
            ((EnemyState*)state)->controlFlags =
                ((EnemyState*)state)->controlFlags & ~(u64)BADDIE_CONTROL_PATH_FOLLOW;
        }
        if (((EnemyState*)state)->userData2 == 0)
        {
            crawler_checkNearbyActive(obj, state);
        }
        ((EnemyState*)state)->userData1 = 0;
    }

    cap = 0.0f;
    if (((EnemyState*)state)->crawler.emergeTimer != cap && ((EnemyState*)state)->familyData.crawler.reactStep != 0)
    {
        ((EnemyState*)state)->crawler.emergeTimer = ((EnemyState*)state)->crawler.emergeTimer - timeDelta;
        if (((EnemyState*)state)->crawler.emergeTimer <= cap)
        {
            ((EnemyState*)state)->crawler.emergeTimer = cap;
            ((EnemyState*)state)->controlFlags |= (u64)BADDIE_CONTROL_SEQUENCE_DRIVEN;
            ((EnemyState*)state)->familyData.crawler.flagsC = seq[((EnemyState*)state)->familyData.crawler.reactStep].flagC;
            obj->hitVolumeIndex = ((EnemyState*)state)->familyData.crawler.flagsC & 1;
            ((EnemyState*)state)->familyData.crawler.reactStep = seq[((EnemyState*)state)->familyData.crawler.reactStep].nextA;
        }
        if ((((EnemyState*)state)->controlFlags & (BADDIE_CONTROL_JUST_TRIGGERED | BADDIE_CONTROL_SEQUENCE_DRIVEN)) ==
            0)
        {
            return;
        }
    }

    {
        u32 flags = ((EnemyState*)state)->controlFlags;
        if ((flags & BADDIE_CONTROL_PATH_FOLLOW) != 0)
        {
            int count = enemy_findNearbyEnemies(obj, 250.0f, 1, 0x28, gCrawlerNearbyObjectBuffer);
            if (count >= 1 && (f32)gCrawlerNearbyObjectBuffer[0].dist <= 250.0f)
            {
                f32* dp = dv;
                int rel;
                u16 oct;
                dp[0] = obj->anim.worldPosX - gCrawlerNearbyObjectBuffer[0].obj->anim.worldPosX;
                dp[1] = obj->anim.worldPosY - gCrawlerNearbyObjectBuffer[0].obj->anim.worldPosY;
                dp[2] = obj->anim.worldPosZ - gCrawlerNearbyObjectBuffer[0].obj->anim.worldPosZ;
                rel = (getAngle(-dp[0], -dp[2]) & 0xffff) - ((int)*(s16*)obj & 0xffffu);
                if (rel > 0x8000)
                {
                    rel = rel - 0xffff;
                }
                if (rel < -0x8000)
                {
                    rel = rel + 0xffff;
                }
                oct = ((u32)rel & 0xffff) >> 13;
                if (oct == 3 || oct == 4)
                {
                    scale = (f32)gCrawlerNearbyObjectBuffer[0].dist / 250.0f;
                }
                else if (oct == 0 || oct == 7)
                {
                    scale = 2.0f * (1.0f - (f32)gCrawlerNearbyObjectBuffer[0].dist / 250.0f) + 1.0f;
                }
            }
            {
                f32 dx = base->posX - obj->anim.localPosX;
                f32 dz = base->posZ - obj->anim.localPosZ;
                f32 dist = sqrtf(dx * dx + dz * dz);
                if (dist > 160.0f)
                {
                    dist = 160.0f;
                }
                {
                    f32 ratio = (160.0f - dist) / 160.0f;
                    ((EnemyState*)state)->pathSpeed = scale * (ratio * ((EnemyState*)state)->pathStep);
                }
                if (((EnemyState*)state)->pathSpeed < 0.25f)
                {
                    ((EnemyState*)state)->pathSpeed = 0.25f;
                }
            }
            if ((Curve_AdvanceAlongPath(&base->curve, ((EnemyState*)state)->pathSpeed) != 0 ||
                 base->atSegmentEnd != 0) &&
                (*gRomCurveInterface)->goNextPoint(base) != 0 &&
                (*gRomCurveInterface)
                        ->initCurve(*(RomCurveWalker**)state, obj, 700.0f, (int*)&gCrawlerCurveInitData, -1) != 0)
            {
                ((EnemyState*)state)->controlFlags =
                    ((EnemyState*)state)->controlFlags & ~(u64)BADDIE_CONTROL_PATH_FOLLOW;
            }
            if ((((EnemyState*)state)->familyData.crawler.flagsD & 0xa) == 0)
            {
                f32 t;
                f32 diff;
                f32 a;
                diff = (f32)(int)(((getAngle(base->tangentX, base->tangentZ) & 0xffff) + 0x8000) -
                                  ((int)*(s16*)obj & 0xffffu));
                if (diff > 32768.0f)
                {
                    diff = -65535.0f + diff;
                }
                if (diff < -32768.0f)
                {
                    diff = 65535.0f + diff;
                }
                t = (((EnemyState*)state)->pathStep * scale - ((EnemyState*)state)->pathSpeed) / 60.0f;
                a = diff >= 0.0f ? diff : -diff;
                /* 0x308 = EnemyState.animPlaySpeed; kept raw here - the typed
                 * member spelling shifts bytes off the u8* state param. */
                *(f32*)(state + 0x308) = t * (1.0f - a / 65535.0f);
                if (*(f32*)(state + 0x308) < 0.005f)
                {
                    *(f32*)(state + 0x308) = 0.005f;
                }
                else if (*(f32*)(state + 0x308) > 0.05f)
                {
                    *(f32*)(state + 0x308) = 0.05f;
                }
            }
            if ((((EnemyState*)state)->controlFlags &
                 (BADDIE_CONTROL_JUST_TRIGGERED | BADDIE_CONTROL_SEQUENCE_DRIVEN)) != 0)
            {
                ((EnemyState*)state)->familyData.crawler.flagsD = ((EnemyState*)state)->familyData.crawler.flagsD & ~0x20;
                if (((EnemyState*)state)->familyData.crawler.reactStep != 0)
                {
                    baddieSetMove(obj, (int)state, seq[((EnemyState*)state)->familyData.crawler.reactStep].moveId,
                                   seq[((EnemyState*)state)->familyData.crawler.reactStep].spd, 0,
                                   seq[((EnemyState*)state)->familyData.crawler.reactStep].mask & 0xff);
                    ((EnemyState*)state)->familyData.crawler.flagsC = seq[((EnemyState*)state)->familyData.crawler.reactStep].flagC;
                    obj->hitVolumeIndex = ((EnemyState*)state)->familyData.crawler.flagsC & 1;
                    ((EnemyState*)state)->familyData.crawler.reactStep = seq[((EnemyState*)state)->familyData.crawler.reactStep].next9;
                }
                else
                {
                    f32* dp2 = dv;
                    int rel2;
                    u16 oct2;
                    u8 mv;
                    dp2[0] = obj->anim.worldPosX - base->posX;
                    dp2[1] = obj->anim.worldPosY - base->posY;
                    dp2[2] = obj->anim.worldPosZ - base->posZ;
                    rel2 = (getAngle(-dp2[0], -dp2[2]) & 0xffff) - ((int)*(s16*)obj & 0xffffu);
                    if (rel2 > 0x8000)
                    {
                        rel2 = rel2 - 0xffff;
                    }
                    if (rel2 < -0x8000)
                    {
                        rel2 = rel2 + 0xffff;
                    }
                    oct2 = ((u32)rel2 & 0xffff) >> 13;
                    i = oct2;
                    mv = tC[i].moveId;
                    if (mv == 0)
                    {
                        ((EnemyState*)state)->familyData.crawler.flagsD = ((EnemyState*)state)->familyData.crawler.flagsD & ~0x18;
                        {
                            f32 v = ((EnemyState*)state)->pathSpeed;
                            int j = ((EnemyState*)state)->userData2;
                            if (v > gCrawlerSpeedThresholds.speeds[j][0])
                            {
                                ((EnemyState*)state)->rootMotionFlags = 1;
                                ObjAnim_SetCurrentMove((u32)obj, *(u8*)(t0 + 0x2c), 0.0f, 0);
                            }
                            else if (v > gCrawlerSpeedThresholds.speeds[j][1])
                            {
                                ((EnemyState*)state)->rootMotionFlags = 1;
                                ObjAnim_SetCurrentMove((u32)obj, *(u8*)(t0 + 0x20), 0.0f, 0);
                            }
                            else if (v > gCrawlerSpeedThresholds.speeds[j][2])
                            {
                                ((EnemyState*)state)->rootMotionFlags = 1;
                                ObjAnim_SetCurrentMove((u32)obj, *(u8*)(t0 + 0x14), 0.0f, 0);
                            }
                            else
                            {
                                ((EnemyState*)state)->rootMotionFlags = 1;
                                *(f32*)(state + 0x308) = 0.01f;
                                ObjAnim_SetCurrentMove((u32)obj, *(u8*)(t0 + 8), 0.0f, 0);
                                ((EnemyState*)state)->pathSpeed = 0.0f;
                            }
                        }
                    }
                    else
                    {
                        baddieSetMove(obj, (int)state, mv, tC[i].spd, 0, tC[i].mode);
                        ((EnemyState*)state)->familyData.crawler.flagsD = ((EnemyState*)state)->familyData.crawler.flagsD | 8;
                    }
                }
            }
            if ((((EnemyState*)state)->rootMotionFlags & 8) == 0 && (((EnemyState*)state)->familyData.crawler.flagsD & 0x10) == 0)
            {
                baddieTurnTowardPoint(obj, (int)state, base->posX, base->posZ, 0xf, 0);
            }
        }
        else if ((flags & 0xc0000000) != 0)
        {
            i = randomGetRange(1, t8[0].moveId) & 0xff;
            baddieSetMove(obj, (int)state, t8[i].moveId, t8[i].spd, 0, t8[i].mode);
        }
    }
    crawlerPlayMoveEventFx(obj, state);
}

void crawler_updateB(GameObject* obj, u8* state)
{
    CrawlerDescriptor* d = (CrawlerDescriptor*)gCrawlerDescriptorTable;
    CrawlerSeq12* t10 = d[((EnemyState*)state)->userData2].tbl10;
    CrawlerSeq12* t8 = d[((EnemyState*)state)->userData2].tbl8;
    CrawlerSeq12* tC = d[((EnemyState*)state)->userData2].tblC;
    CrawlerSeq16* seq = d[((EnemyState*)state)->userData2].seq;
    CrawlerSeq12* t4 = d[((EnemyState*)state)->userData2].tbl4;
    CrawlerSeq12* t18 = d[((EnemyState*)state)->userData2].tbl18;
    f32 cap;
    int count;
    int i;
    f32 dv[3];

    if (((EnemyState*)state)->trackedObj != NULL &&
        ((GameObject*)((EnemyState*)state)->trackedObj)->anim.classId == 1)
    {
        requestGalleonBattleMusic();
    }

    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_JUST_TRIGGERED) != 0)
    {
        if (((EnemyState*)state)->userData2 == 0)
        {
            (*gCameraInterface)->loadTriggeredCamAction(0, 0x6c, 0);
        }
        ((EnemyState*)state)->familyData.crawler.flagsD = ((EnemyState*)state)->familyData.crawler.flagsD | 0x10;
        ((EnemyState*)state)->userData1 = 0;
        if (obj->anim.romDefNo == FIRECRAWLER_SEQID_FIRECRAWLER)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_eggsnatch_var);
            if (obj->childObjs[0] != NULL)
            {
                firepipe_clearLinkedUpdateFlag(obj->childObjs[0]);
            }
        }
    }

    cap = 0.0f;
    if (((EnemyState*)state)->crawler.emergeTimer != cap && ((EnemyState*)state)->familyData.crawler.reactStep != 0)
    {
        ((EnemyState*)state)->crawler.emergeTimer = ((EnemyState*)state)->crawler.emergeTimer - timeDelta;
        if (((EnemyState*)state)->crawler.emergeTimer <= cap)
        {
            ((EnemyState*)state)->crawler.emergeTimer = cap;
            ((EnemyState*)state)->controlFlags |= (u64)BADDIE_CONTROL_SEQUENCE_DRIVEN;
            ((EnemyState*)state)->familyData.crawler.flagsC = seq[((EnemyState*)state)->familyData.crawler.reactStep].flagC;
            obj->hitVolumeIndex = ((EnemyState*)state)->familyData.crawler.flagsC & 1;
            ((EnemyState*)state)->familyData.crawler.reactStep = seq[((EnemyState*)state)->familyData.crawler.reactStep].nextA;
        }
    }

    count = enemy_findNearbyEnemies(obj, 180.0f, 1, 0x28, gCrawlerNearbyObjectBuffer);
    if (count >= 1)
    {
        if ((((EnemyState*)state)->familyData.crawler.flagsD & 0x20) == 0 ||
            (((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            if (((EnemyState*)state)->familyData.crawler.reactStep != 0)
            {
                baddieSetMove(obj, (int)state, seq[((EnemyState*)state)->familyData.crawler.reactStep].moveId,
                               seq[((EnemyState*)state)->familyData.crawler.reactStep].spd, 0, seq[((EnemyState*)state)->familyData.crawler.reactStep].mask & 0xff);
                ((EnemyState*)state)->familyData.crawler.flagsC = seq[((EnemyState*)state)->familyData.crawler.reactStep].flagC;
                obj->hitVolumeIndex = ((EnemyState*)state)->familyData.crawler.flagsC & 1;
                ((EnemyState*)state)->familyData.crawler.reactStep = seq[((EnemyState*)state)->familyData.crawler.reactStep].next9;
            }
            else
            {
                f32* dp = dv;
                int rel;
                u16 oct;
                dp[0] = obj->anim.worldPosX - gCrawlerNearbyObjectBuffer[0].obj->anim.worldPosX;
                dp[1] = obj->anim.worldPosY - gCrawlerNearbyObjectBuffer[0].obj->anim.worldPosY;
                dp[2] = obj->anim.worldPosZ - gCrawlerNearbyObjectBuffer[0].obj->anim.worldPosZ;
                rel = (getAngle(-dp[0], -dp[2]) & 0xffff) - ((int)*(s16*)obj & 0xffffu);
                if (rel > 0x8000)
                {
                    rel = rel - 0xffff;
                }
                if (rel < -0x8000)
                {
                    rel = rel + 0xffff;
                }
                oct = ((u32)rel & 0xffff) >> 13;
                if (oct != 0 && oct < 7)
                {
                    if (oct < 3 || oct > 4)
                    {
                        u8 mv;
                        i = ((EnemyState*)state)->turnOctant;
                        mv = tC[i].moveId;
                        if (mv == 0)
                        {
                            int i2 = ((EnemyState*)state)->familyData.crawler.moveChainIndex;

                            baddieSetMove(obj, (int)state, t4[i2].moveId, t4[i2].spd, 0, t4[i2].mode);
                            ((EnemyState*)state)->familyData.crawler.moveChainIndex = t4[((EnemyState*)state)->familyData.crawler.moveChainIndex].next;
                        }
                        else
                        {
                            baddieSetMove(obj, (int)state, mv, tC[i].spd, 0, tC[i].mode);
                        }
                    }
                    else
                    {
                        i = randomGetRange(1, t8[0].moveId) & 0xff;
                        baddieSetMove(obj, (int)state, t8[i].moveId, t8[i].spd, 0, t8[i].mode);
                    }
                }
                else
                {
                    baddieSetMove(obj, (int)state, t10[0].moveId, t10[0].spd, 0, t10[0].mode);
                }
                ((EnemyState*)state)->familyData.crawler.flagsD = ((EnemyState*)state)->familyData.crawler.flagsD | 0x20;
                ((EnemyState*)state)->familyData.crawler.flagsD = ((EnemyState*)state)->familyData.crawler.flagsD & ~0x10;
            }
        }
    }
    else
    {
        if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
        {
            ((EnemyState*)state)->familyData.crawler.flagsD = ((EnemyState*)state)->familyData.crawler.flagsD & ~0x30;
            if (obj->anim.romDefNo == FIRECRAWLER_SEQID_FIRECRAWLER &&
                obj->childObjs[0] != NULL)
            {
                firepipe_clearLinkedUpdateFlag(obj->childObjs[0]);
            }
            if (((EnemyState*)state)->familyData.crawler.reactStep != 0)
            {
                baddieSetMove(obj, (int)state, seq[((EnemyState*)state)->familyData.crawler.reactStep].moveId,
                               seq[((EnemyState*)state)->familyData.crawler.reactStep].spd, 0, seq[((EnemyState*)state)->familyData.crawler.reactStep].mask & 0xff);
                ((EnemyState*)state)->familyData.crawler.flagsC = seq[((EnemyState*)state)->familyData.crawler.reactStep].flagC;
                obj->hitVolumeIndex = ((EnemyState*)state)->familyData.crawler.flagsC & 1;
                ((EnemyState*)state)->familyData.crawler.reactStep = seq[((EnemyState*)state)->familyData.crawler.reactStep].next9;
            }
            else
            {
                int i2;
                CrawlerSeq12* q;
                if ((((EnemyState*)state)->controlFlags &
                     (q = &t4[i2 = ((EnemyState*)state)->familyData.crawler.moveChainIndex])->mask) != 0)
                {
                    u8 mv;
                    i = ((EnemyState*)state)->turnOctant;
                    mv = tC[i].moveId;
                    if (mv == 0)
                    {
                        baddieSetMove(obj, (int)state, q->moveId, t4[i2].spd, 0, q->mode);
                    }
                    else
                    {
                        baddieSetMove(obj, (int)state, mv, tC[i].spd, 0, tC[i].mode);
                    }
                }
                else
                {
                    u8 mv;
                    i = ((EnemyState*)state)->turnOctant;
                    mv = tC[i].moveId;
                    if (mv == 0)
                    {
                        int i4 = randomGetRange(1, t8[0].moveId) & 0xff;
                        baddieSetMove(obj, (int)state, t8[i4].moveId, t8[i4].spd, 0, t8[i4].mode);
                    }
                    else
                    {
                        baddieSetMove(obj, (int)state, mv, tC[i].spd, 0, tC[i].mode);
                    }
                }
                {
                    ((EnemyState*)state)->familyData.crawler.moveChainIndex = t4[((EnemyState*)state)->familyData.crawler.moveChainIndex].next;
                }
            }
        }
    }

    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumePriority = 0;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumeId = 0;
    {
        int j = 1;
        u8* p = (u8*)t18 + 0xc;
        int c;
        for (c = *(u8*)((char*)t18 + 8); c >= 1; c--)
        {
            if (obj->anim.currentMove == *(u8*)(p + 8))
            {
                p = (u8*)t18 + j * 0xc;
                ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumePriority =
                    (s8) * (int*)(p + 4);
                ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumeId = (s8) * (u8*)(p + 9);
                if (((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumePriority == 0x1f)
                {
                    ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x40;
                }
                else
                {
                    ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 & ~0x40LL;
                }
                break;
            }
            p += 0xc;
            j += 1;
        }
    }

    if ((((EnemyState*)state)->rootMotionFlags & 8) == 0 && (((EnemyState*)state)->familyData.crawler.flagsD & 0x10) == 0)
    {
        baddieTurnTowardPoint(obj, (int)state,
                    ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX,
                    ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ, 0x1e, 0);
    }
    crawlerPlayMoveEventFx(obj, state);
}

void crawler_update(GameObject* obj, u8* state)
{
    typedef struct
    {
        u8 pad[0xc];
        CrawlerSeq12* tC;
        CrawlerSeq12* t10;
        CrawlerSeq16* t14;
        CrawlerSeq12* t18;
        u8 pad2[4];
    } CrawlerDescL;
    CrawlerDescL* d = (CrawlerDescL*)gCrawlerDescriptorTable;
    CrawlerSeq12* t9 = d[((EnemyState*)state)->userData2].t10;
    CrawlerSeq12* t8 = d[((EnemyState*)state)->userData2].t18;
    CrawlerSeq12* t7 = d[((EnemyState*)state)->userData2].tC;
    CrawlerSeq16* t6 = d[((EnemyState*)state)->userData2].t14;
    f32 cap;
    int i;
    u8* p;
    int j;
    int n;

    if (((EnemyState*)state)->trackedObj != NULL &&
        ((GameObject*)((EnemyState*)state)->trackedObj)->anim.classId == 1)
    {
        requestGalleonBattleMusic();
    }

    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_JUST_TRIGGERED) != 0)
    {
        if (((EnemyState*)state)->userData2 == 0)
        {
            (*gCameraInterface)->loadTriggeredCamAction(0, 0x6c, 0);
        }
        if (obj->anim.romDefNo == FIRECRAWLER_SEQID_FIRECRAWLER && obj->childObjs[0] != NULL)
        {
            firepipe_clearLinkedUpdateFlag(obj->childObjs[0]);
        }
        ((EnemyState*)state)->familyData.crawler.flagsD = ((EnemyState*)state)->familyData.crawler.flagsD | 0x10;
    }

    cap = 0.0f;
    if (((EnemyState*)state)->crawler.emergeTimer != cap && ((EnemyState*)state)->familyData.crawler.reactStep != 0)
    {
        ((EnemyState*)state)->crawler.emergeTimer -= timeDelta;
        if (((EnemyState*)state)->crawler.emergeTimer <= cap)
        {
            ((EnemyState*)state)->crawler.emergeTimer = cap;
            ((EnemyState*)state)->controlFlags |= (u64)BADDIE_CONTROL_SEQUENCE_DRIVEN;
            ((EnemyState*)state)->familyData.crawler.flagsC = t6[((EnemyState*)state)->familyData.crawler.reactStep].flagC;
            obj->hitVolumeIndex = ((EnemyState*)state)->familyData.crawler.flagsC & 1;
            ((EnemyState*)state)->familyData.crawler.reactStep = t6[((EnemyState*)state)->familyData.crawler.reactStep].nextA;
        }
    }

    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        ((EnemyState*)state)->familyData.crawler.flagsD = ((EnemyState*)state)->familyData.crawler.flagsD & ~0x30;
        if (obj->anim.romDefNo == FIRECRAWLER_SEQID_FIRECRAWLER && obj->childObjs[0] != NULL)
        {
            firepipe_clearLinkedUpdateFlag(obj->childObjs[0]);
        }
        if (((EnemyState*)state)->familyData.crawler.reactStep != 0)
        {
            baddieSetMove(obj, (int)state, t6[((EnemyState*)state)->familyData.crawler.reactStep].moveId, t6[((EnemyState*)state)->familyData.crawler.reactStep].spd, 0,
                           t6[((EnemyState*)state)->familyData.crawler.reactStep].mask & 0xff);
            ((EnemyState*)state)->familyData.crawler.flagsC = t6[((EnemyState*)state)->familyData.crawler.reactStep].flagC;
            obj->hitVolumeIndex = ((EnemyState*)state)->familyData.crawler.flagsC & 1;
            ((EnemyState*)state)->familyData.crawler.reactStep = t6[((EnemyState*)state)->familyData.crawler.reactStep].next9;
        }
        else
        {
            i = ((EnemyState*)state)->turnOctant;
            if (t7[i].moveId == 0)
            {
                if (((EnemyState*)state)->targetDist >= 0x50)
                {
                    ((EnemyState*)state)->userData1 = 0;
                }
                enemy_findNearbyEnemies(obj, 250.0f, 6, 0x28, gCrawlerNearbyObjectBuffer);
                if ((((EnemyState*)state)->controlFlags & t9[((EnemyState*)state)->userData1].mask) == 0 &&
                    t9[((EnemyState*)state)->userData1].next != 0)
                {
                    ((EnemyState*)state)->userData1 = t9[((EnemyState*)state)->userData1].next;
                }
                baddieSetMove(obj, (int)state, t9[((EnemyState*)state)->userData1].moveId,
                               t9[((EnemyState*)state)->userData1].spd, 0,
                               t9[((EnemyState*)state)->userData1].mode);
                ((EnemyState*)state)->userData1 = t9[((EnemyState*)state)->userData1].next;
            }
            else
            {
                baddieSetMove(obj, (int)state, t7[i].moveId, t7[i].spd, 0, t7[i].mode);
            }
        }
    }

    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumePriority = 0;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumeId = 0;
    j = 1;
    p = (u8*)t8 + 0xc;
    n = *(u8*)((char*)t8 + 8);
    for (; j <= n; j++)
    {
        if (obj->anim.currentMove == *(u8*)(p + 8))
        {
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumePriority =
                (s8) * (int*)((char*)t8 + j * 0xc + 4);
            ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumeId =
                (s8) * (u8*)((char*)t8 + j * 0xc + 9);
            if (((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumePriority == 0x1f)
            {
                ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x40;
            }
            else
            {
                ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 & ~0x40LL;
            }
            break;
        }
        p += 0xc;
    }

    if ((((EnemyState*)state)->rootMotionFlags & 8) == 0 && (((EnemyState*)state)->familyData.crawler.flagsD & 0x10) == 0)
    {
        baddieTurnTowardPoint(obj, (int)state,
                    ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX,
                    ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ, 0x1e, 0);
    }
    crawlerPlayMoveEventFx(obj, state);
}

/* crawler_initModelVariant: crawler-family variant init. Dispatches on obj->modelType
 * (offset 0x46): values 0x6a2/0x6a3/0x6a4 each pick a different float +
 * byte tuple to seed state[0x2a8..0x322]. The trailing block sets
 * shared state floats and computes obj->anim.rootMotionScale from params[0x28]. */
void crawler_initModelVariant(GameObject* obj, u8* state)
{
    u8* params = (u8*)obj->anim.placementData;
    ((EnemyState*)state)->flags2E4 = 0xb;
    ((EnemyState*)state)->flags2E4 |= 0x400b0LL;
    ((EnemyState*)state)->flags2E4 |= 0x40001040LL;
    switch (obj->anim.romDefNo)
    {
    case FIRECRAWLER_SEQID_REDEYE:
        ((EnemyState*)state)->sightRange = 150.0f;
        ((EnemyState*)state)->aggroRange = 250.0f;
        ((EnemyState*)state)->current = 0x1e;
        ((EnemyState*)state)->userData2 = 0;
        ((EnemyState*)state)->moveId0 = 9;
        ((EnemyState*)state)->moveSpeedScale0 = 3.0f;
        ((EnemyState*)state)->moveId1 = 0xc;
        ((EnemyState*)state)->moveSpeedScale1 = 1.25f;
        ((EnemyState*)state)->moveId2 = 9;
        ((EnemyState*)state)->moveSpeedScale2 = 3.0f;
        ((EnemyState*)state)->flags2E4 |= 0x400;
        break;
    case FIRECRAWLER_SEQID_FIRECRAWLER:
        ((EnemyState*)state)->sightRange = 130.0f;
        ((EnemyState*)state)->aggroRange = 250.0f;
        ((EnemyState*)state)->current = 0x32;
        ((EnemyState*)state)->userData2 = 1;
        ((EnemyState*)state)->moveId0 = 0xe;
        ((EnemyState*)state)->moveSpeedScale0 = 3.0f;
        ((EnemyState*)state)->moveId1 = 0xd;
        ((EnemyState*)state)->moveSpeedScale1 = 1.25f;
        ((EnemyState*)state)->moveId2 = 0xe;
        ((EnemyState*)state)->moveSpeedScale2 = 3.0f;
        ((EnemyState*)state)->flags2E4 |= 0xc00;
        break;
    case FIRECRAWLER_SEQID_SHADOWHUNTER:
        ((EnemyState*)state)->sightRange = 120.0f;
        ((EnemyState*)state)->aggroRange = 240.0f;
        ((EnemyState*)state)->current = 0xf;
        ((EnemyState*)state)->userData2 = 2;
        ((EnemyState*)state)->moveId0 = 0xd;
        ((EnemyState*)state)->moveSpeedScale0 = 3.0f;
        ((EnemyState*)state)->moveId1 = 0x10;
        ((EnemyState*)state)->moveSpeedScale1 = 1.25f;
        ((EnemyState*)state)->moveId2 = 0xd;
        ((EnemyState*)state)->moveSpeedScale2 = 3.0f;
        ((EnemyState*)state)->flags2E4 |= 0xc00;
        break;
    }
    ((EnemyState*)state)->animPlaySpeed = 0.005f;
    ((EnemyState*)state)->gravity = 0.17f;
    ((EnemyState*)state)->drag = 0.97f;
    ((EnemyState*)state)->pathStep *= 10.0f;
    if ((s8)params[0x2e] != -1)
    {
        ((EnemyState*)state)->controlFlags |= 1;
    }
    obj->anim.rootMotionScale = 0.5f + ((f32)(s32)(s8)params[0x28] / 127.0f);
}

extern int gHagabonMK2CurveInitData[2];

extern void* gCrawlerModelChainIds[];


static inline void crawler_createEngineLight(GameObject* obj, u8* state)
{
    if (((EnemyState*)state)->modelLight == NULL)
    {
        ((EnemyState*)state)->modelLight = objCreateLight(NULL, 1);
    }
    if (((EnemyState*)state)->modelLight != NULL)
    {
        modelLightStruct_setLightKind(((EnemyState*)state)->modelLight, MODEL_LIGHT_KIND_POINT);
        modelLightStruct_setPosition(((EnemyState*)state)->modelLight, obj->anim.localPosX,
                                     obj->anim.localPosY, obj->anim.localPosZ);
        modelLightStruct_setDiffuseColor(((EnemyState*)state)->modelLight, 0xc0, 0x40, 0xff, 0xff);
        modelLightStruct_setSpecularColor(((EnemyState*)state)->modelLight, 0xc0, 0x40, 0xff, 0xff);
        modelLightStruct_setDistanceAttenuation(((EnemyState*)state)->modelLight, 100.0f, 150.0f);
        lightSetField4D(((EnemyState*)state)->modelLight, 1);
        modelLightStruct_setEnabled(((EnemyState*)state)->modelLight, 1, 0.5f);
        modelLightStruct_startColorFade(((EnemyState*)state)->modelLight, 0, 0);
        modelLightStruct_setAffectsAabbLightSelection(((EnemyState*)state)->modelLight, 0);
    }
}

void crawler_rotateVectorYaw(int unused1, int* unused2, f32* vec, int unused3, int nodeIndex, f32 phase)
{
    f32 mtx[12];
    f32 a;
    a = 0.02f * mathCosfHighPrecision(0.08f * phase - 1.1f * (f32)nodeIndex);
    PSMTXRotRad((MtxPtr)mtx, 'y', a);
    PSMTXMultVecSR((MtxPtr)mtx, (Vec*)vec, (Vec*)vec);
}

void hagabonMK2_stopLoopSfx(int obj, u8* state)
{
    Sfx_StopFromObject(obj, SFXTRIG_baddie_rach_death);
}

void hagabonMK2_updateWhileFrozen(int obj, u8* st, int unused, int cmd, int wpad0, int wpad1, Vec* wpad2, int wpad3)
{
    int objI = (int)obj;
    if (cmd == 0x11)
    {
    }
    else if (cmd == 0x10)
    {
        ((EnemyState*)st)->flags2E8 |= 0x20;
    }
    else
    {
        ((EnemyState*)st)->flags2E8 |= 0x8;
        Sfx_StopFromObject(objI, SFXTRIG_baddie_rach_death);
        Sfx_PlayFromObject(obj, SFXTRIG_baddie_eba_leavesopen);
        ((EnemyState*)st)->current = 0;
    }
}

void hagabonMK2_updateB(GameObject* obj, u8* state)
{
    RomCurveWalker* base = *(RomCurveWalker**)state;
    f32 spd;
    f32 cap;
    CrawlerSfxParams sp;
    f32 dv[3];
    int i;

    if (((EnemyState*)state)->crawler.warpTimer)
    {
        cap = 0.0f;
        ((EnemyState*)state)->crawler.warpTimer = ((EnemyState*)state)->crawler.warpTimer - timeDelta;
        if (((EnemyState*)state)->crawler.warpTimer <= cap)
        {
            ((EnemyState*)state)->crawler.warpTimer = cap;
        }
    }
    ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x100;
    sp.x = 0.0f;
    sp.y = 4.0f;
    sp.z = 0.0f;
    sp.vol = 1.1f;
    sp.sfxId = 0x605;
    if ((obj->objectFlags & FIRECRAWLER_OBJFLAG_RENDERED) != 0)
    {
        (*gPartfxInterface)->spawnObject(obj, 1999, &sp, 2, -1, NULL);
        if (((EnemyState*)state)->modelLight == NULL)
        {
            crawler_createEngineLight(obj, state);
        }
        else
        {
            modelLightStruct_setPosition(((EnemyState*)state)->modelLight, obj->anim.localPosX,
                                         obj->anim.localPosY, obj->anim.localPosZ);
        }
    }

    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_JUST_TRIGGERED) != 0)
    {
        CrawlerSeq12* sq = (CrawlerSeq12*)gCrawlerSeqTable;
        ((EnemyState*)state)->userData1 = sq[((EnemyState*)state)->userData1].mode;
        ((EnemyState*)state)->crawler.emergeTimer = 200.0f;
        Sfx_StopFromObject((int)obj, SFXTRIG_baddie_rach_death);
    }

    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_PATH_FOLLOW) != 0)
    {
        f32* dp = dv;
        f32 t;
        dp[0] = base->posX - obj->anim.worldPosX;
        dp[1] = base->posY - obj->anim.worldPosY;
        dp[2] = base->posZ - obj->anim.worldPosZ;
        ((EnemyState*)state)->crawler.distToCurve = sqrtf(dp[2] * dp[2] + (dp[0] * dp[0] + dp[1] * dp[1]));
        if (((EnemyState*)state)->crawler.distToCurve < 100.0f && !((EnemyState*)state)->crawler.warpTimer)
        {
            ((EnemyState*)state)->flags2E4 = ((EnemyState*)state)->flags2E4 & ~0x10000LL;
        }
        t = 1.0f - ((EnemyState*)state)->crawler.distToCurve / 400.0f;
        if (t < 0.0f)
        {
            t = 0.0f;
        }
        else if (t > 1.0f)
        {
            t = 1.0f;
        }
        if ((Curve_AdvanceAlongPath(&base->curve, ((EnemyState*)state)->pathStep * t) != 0 ||
             base->atSegmentEnd != 0) &&
            (*gRomCurveInterface)->goNextPoint(base) != 0 &&
            (*gRomCurveInterface)->initCurve(*(RomCurveWalker**)state, obj, 700.0f, (int*)&gHagabonMK2CurveInitData, -1) != 0)
        {
            ((EnemyState*)state)->controlFlags =
                ((EnemyState*)state)->controlFlags & ~(u64)BADDIE_CONTROL_PATH_FOLLOW;
        }
        sidekickToy_accelerateTowardTarget3D(obj, base->posX, base->posY, base->posZ, 60.0f,
                                             0.05f, 5.0f, ((EnemyState*)state)->drag);
    }

    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        CrawlerSeq12* sq = (CrawlerSeq12*)gCrawlerSeqTable;
        i = ((EnemyState*)state)->userData1 * 0xc;
        baddieSetMove(obj, (int)state, *(u8*)(gCrawlerSeqTable + i + 8),
                    *(f32*)((int)gCrawlerSeqTable + i), 0, 0);
        ((EnemyState*)state)->userData1 = sq[((EnemyState*)state)->userData1].next;
    }

    if (((EnemyState*)state)->crawler.engineTimer > 0.0f)
    {
        ((EnemyState*)state)->crawler.engineTimer = -(18.2f * timeDelta - ((EnemyState*)state)->crawler.engineTimer);
        *(s16*)obj = ((EnemyState*)state)->crawler.engineTimer * timeDelta + (f32)(int)*(s16*)obj;
    }
    else
    {
        f32 ratio;
        ((EnemyState*)state)->crawler.engineTimer = 0.0f;
        spd = 1.0f - (((EnemyState*)state)->crawler.emergeTimer - 15.0f) / 185.0f;
        if (spd < 0.0001f)
        {
            spd = 0.0001f;
        }
        else if (spd > 1.0f)
        {
            spd = 1.0f;
        }
        if (((EnemyState*)state)->crawler.emergeTimer > 15.0f)
        {
            ((EnemyState*)state)->crawler.emergeTimer -= timeDelta;
        }
        else
        {
            ((EnemyState*)state)->crawler.emergeTimer = 15.0f;
        }
        ratio = sqrtf(obj->anim.velocityX * obj->anim.velocityX +
                      obj->anim.velocityZ * obj->anim.velocityZ) /
                60.0f;
        if (ratio < 0.0f)
        {
            ratio = 0.0f;
        }
        else if (ratio > 1.0f)
        {
            ratio = 1.0f;
        }
        {
            f32 t = 6370.0f * spd;
            ratio *= t * timeDelta;
        }
        obj->anim.rotY = (f32)(int)obj->anim.rotY - ratio;
        baddieTurnTowardLookDir(obj, state, (int)((EnemyState*)state)->crawler.emergeTimer, 10.0f * spd,
                    0.0f, 1);
    }

    {
        f32 pw = powfBitEstimate(((EnemyState*)state)->drag, timeDelta);
        obj->anim.rotY = (f32)obj->anim.rotY * pw;
        pw = powfBitEstimate(((EnemyState*)state)->drag, timeDelta);
        obj->anim.rotZ = (f32)obj->anim.rotZ * pw;
    }

    if ((int)randomGetRange(0, 0x2ee) == 0)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_eba);
    }

    if (((EnemyState*)state)->crawler.engineTimer > 0.0f)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_rach_death);
        {
            f32 t = ((EnemyState*)state)->crawler.engineTimer;
            Sfx_SetObjectSfxVolume((u32)obj, SFXTRIG_baddie_rach_death,
                                   (127.0f * t) / 2184.0f,
                                   t / 2184.0f);
        }
    }
    else
    {
        Sfx_StopFromObject((int)obj, SFXTRIG_baddie_rach_death);
    }

    {
        s16 t;
        if (*(GameObject**)&((EnemyState*)state)->lastHitObject != NULL &&
            ((t = (*(GameObject**)&((EnemyState*)state)->lastHitObject)->anim.romDefNo) == 0x1f || t == 0))
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_fball2_c);
        }
    }
}

void hagabonMK2_update(GameObject* obj, u8* state)
{
    RomCurveWalker* base = *(RomCurveWalker**)state;
    f32 d[3];
    CrawlerSfxParams sp;
    int i;
    f32 pw;

    if (*(GameObject**)&((EnemyState*)state)->lastHitObject != NULL && *(GameObject**)&((EnemyState*)state)->lastHitObject == ((EnemyState*)state)->trackedObj)
    {
        ((EnemyState*)state)->flags2E4 |= 0x10000LL;
        ((EnemyState*)state)->crawler.warpTimer = 180.0f;
    }
    ((EnemyState*)state)->flags2E8 = ((EnemyState*)state)->flags2E8 | 0x100;
    sp.x = 0.0f;
    sp.y = 4.0f;
    sp.z = 0.0f;
    sp.vol = 1.1f;
    sp.sfxId = 0x605;
    if ((obj->objectFlags & FIRECRAWLER_OBJFLAG_RENDERED) != 0)
    {
        (*gPartfxInterface)->spawnObject(obj, 1999, &sp, 2, -1, NULL);
        if (((EnemyState*)state)->modelLight == NULL)
        {
            crawler_createEngineLight(obj, state);
        }
        else
        {
            modelLightStruct_setPosition(((EnemyState*)state)->modelLight, obj->anim.localPosX,
                                         obj->anim.localPosY, obj->anim.localPosZ);
        }
    }
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_JUST_TRIGGERED) != 0)
    {
        ((EnemyState*)state)->userData1 = 3;
        ((EnemyState*)state)->controlFlags |= (u64)BADDIE_CONTROL_SEQUENCE_DRIVEN;
    }
    sidekickToy_accelerateTowardTarget3D(
        obj, ((GameObject*)((EnemyState*)state)->trackedObj)->anim.worldPosX,
        60.0f + ((GameObject*)((EnemyState*)state)->trackedObj)->anim.worldPosY,
        ((GameObject*)((EnemyState*)state)->trackedObj)->anim.worldPosZ, 60.0f, 0.025f,
        5.0f, ((EnemyState*)state)->drag);
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        i = ((EnemyState*)state)->userData1 * 0xc;
        baddieSetMove(obj, (int)state, *(u8*)(gCrawlerSeqTable + i + 8),
                    *(f32*)((int)gCrawlerSeqTable + i), 0, 0);
        {
            CrawlerSeq12* sq = (CrawlerSeq12*)gCrawlerSeqTable;
            ((EnemyState*)state)->userData1 = sq[((EnemyState*)state)->userData1].next;
        }
    }
    pw = powfBitEstimate(((EnemyState*)state)->drag, timeDelta);
    obj->anim.rotY = (f32)obj->anim.rotY * pw;
    pw = powfBitEstimate(((EnemyState*)state)->drag, timeDelta);
    obj->anim.rotZ = (f32)obj->anim.rotZ * pw;
    if (((EnemyState*)state)->crawler.engineTimer < 2184.0f)
    {
        ((EnemyState*)state)->crawler.engineTimer = 18.2f * timeDelta + ((EnemyState*)state)->crawler.engineTimer;
    }
    else
    {
        ((EnemyState*)state)->crawler.engineTimer = 2184.0f;
    }
    *(s16*)obj = ((EnemyState*)state)->crawler.engineTimer * timeDelta + (f32)(int)*(s16*)obj;
    ((EnemyState*)state)->crawler.emergeTimer = 200.0f;
    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_PATH_FOLLOW) != 0)
    {
        f32* dp = d;
        dp[0] = base->posX - obj->anim.worldPosX;
        dp[1] = base->posY - obj->anim.worldPosY;
        dp[2] = base->posZ - obj->anim.worldPosZ;
        ((EnemyState*)state)->crawler.distToCurve = sqrtf(dp[2] * dp[2] + (dp[0] * dp[0] + dp[1] * dp[1]));
        if (((EnemyState*)state)->crawler.distToCurve > 400.0f)
        {
            ((EnemyState*)state)->flags2E4 |= 0x10000LL;
            ((EnemyState*)state)->crawler.warpTimer = 0.0f;
        }
    }
    if (((EnemyState*)state)->crawler.engineTimer > 0.0f)
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_rach_death);
        {
            f32 t = ((EnemyState*)state)->crawler.engineTimer;
            Sfx_SetObjectSfxVolume((u32)obj, SFXTRIG_baddie_rach_death,
                                   (127.0f * t) / 2184.0f,
                                   t / 2184.0f);
        }
    }
    else
    {
        Sfx_StopFromObject((int)obj, SFXTRIG_baddie_rach_death);
    }
    if (*(GameObject**)&((EnemyState*)state)->lastHitObject != NULL && ((*(GameObject**)&((EnemyState*)state)->lastHitObject)->anim.romDefNo == 0x1f ||
                                                (*(GameObject**)&((EnemyState*)state)->lastHitObject)->anim.romDefNo == 0))
    {
        Sfx_PlayFromObject((int)obj, SFXTRIG_fball2_c);
    }
}

void hagabonMK2_init(GameObject* obj, EnemyState* st)
{
    st->sightRange = 40.0f;
    st->flags2E4 = 0x405009;
    st->drag = 0.97f;
    *((u8*)st + 0x320) = 0;
    {
        f32 d1 = 1.5f;
        st->moveSpeedScale0 = d1;
        *((u8*)st + 0x321) = 0;
        st->moveSpeedScale1 = 1.0f;
        *((u8*)st + 0x322) = 0;
        st->moveSpeedScale2 = d1;
    }
    st->pathStep *= 3.0f;
    {
        f32* fbase = (f32*)gCrawlerSeqTable;
        u8* bbase = gCrawlerSeqTable;
        u32 idx = st->userData1;
        u32 off = idx * 0xc;
        baddieSetMove(obj, (int)st, bbase[off + 8], *(f32*)((char*)fbase + off), 0, 0);
    }
    ((EnemyState*)st)->crawler.emergeTimer = 15.0f;
    ObjHits_SetHitVolumeMasks(&obj->anim, 0xe, 1, 0xfff);
    ((EnemyState*)st)->tailSimHandle = ObjModelChain_Alloc(gCrawlerModelChainIds, 5);
    ObjModelChain_SetOrigin(((EnemyState*)st)->tailSimHandle, 0.1f, 0.85f, -0.075f);
    st->flags2E8 = st->flags2E8 | 0x100;
    *(int*)((char*)obj + 0x108) = (int)&baddieAfterUpdateBonesCb;
}

extern u8 gSnowwormTurnRates[4];

#define SNOWWORM_SEQID_BABY            0x84b /* "snowworm_ba" - the baby variant of 0x842 "snowworm" */

extern u8* gCrawlerReactionTables[];

extern f32 gCrawlerPi;
extern f32 gCrawlerHalfCircleBams;


void snowworm_spawnProjectile(GameObject* obj)
{
    u8 locked = Obj_IsLoadingLocked();
    if (locked != 0)
    {
        int* setup = (int*)Obj_AllocObjectSetup(0x24, KALDACHOM_SPIT_OBJ);
        ((ObjPlacement*)setup)->posX = obj->anim.localPosX;
        ((ObjPlacement*)setup)->posY = 15.0f + obj->anim.localPosY;
        ((ObjPlacement*)setup)->posZ = obj->anim.localPosZ;
        ((ObjPlacement*)setup)->color[0] = 1;
        ((ObjPlacement*)setup)->color[1] = 4;
        ((ObjPlacement*)setup)->color[3] = 0xff;
        setup = (int*)Obj_SetupObject((ObjPlacement*)setup, 5, -1, -1, 0);
        if (setup != NULL)
        {
            ((GameObject*)setup)->anim.velocityX =
                3.0f * -mathSinf((gCrawlerPi * (f32)*(s16*)obj) / gCrawlerHalfCircleBams);
            ((GameObject*)setup)->anim.velocityY = 0.0f;
            ((GameObject*)setup)->anim.velocityZ =
                3.0f * -mathCosf((gCrawlerPi * (f32)*(s16*)obj) / gCrawlerHalfCircleBams);
        }
    }
}

void snowworm_updateWhileFrozen(int obj, u8* st, int p3, int cmd, int p5, int sub, Vec* wpad0, int wpad1)
{
    u8* base;
    u32 r;

    {
        u8* bbase;
        u32 idx;
        bbase = (u8*)gCrawlerReactionTables;
        idx = ((EnemyState*)st)->phaseAngle;
        bbase = bbase + idx * 8;
        base = *(u8**)(bbase + 4);
    }

    if (cmd == 0x11)
    {
        return;
    }
    if (cmd == 0x10)
    {
        ((EnemyState*)st)->flags2E8 |= 0x20;
        return;
    }
    if (((EnemyState*)st)->turnOctant > 3)
    {
        baddieSetMove((GameObject*)obj, (int)st, 6, 0.5f, 0, 0);
    }
    else
    {
        baddieSetMove((GameObject*)obj, (int)st, 5, 0.5f, 0, 0);
    }
    r = randomGetRange(0, 3);
    ((EnemyState*)st)->userData1 = base[r];
    ((EnemyState*)st)->flags2E8 |= 0x8;
    if (sub > (int)((EnemyState*)st)->current)
    {
        ((EnemyState*)st)->current = 0;
    }
    else
    {
        ((EnemyState*)st)->current = (u16)(((EnemyState*)st)->current - sub);
    }
    if (((EnemyState*)st)->current == 0)
    {
        Sfx_PlayFromObject(obj, SFXTRIG_baddie_eggsnatch_carry2);
    }
    if (cmd == 0x1a)
        return;
    Sfx_PlayFromObject(obj, SFXTRIG_stftest);
}

void crawler_playReactionEffects(GameObject* obj, int* st)
{
    u16 flag = 0;
    switch (obj->anim.currentMove)
    {
    case 2:
        if (((EnemyState*)st)->animEventMask != 0)
        {
            Sfx_PlayFromObjectLimited((u32)obj, SFXTRIG_baddie_blooplaugh3, 2);
        }
        flag = 1;
        break;
    case 3:
        if (((EnemyState*)st)->animEventMask != 0)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_haga_death);
        }
        break;
    case 4:
        if (((EnemyState*)st)->animEventMask != 0)
        {
            if (obj->anim.currentMoveProgress < 0.15f)
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_blooplaugh1);
            }
            else
            {
                Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_rach_call1);
            }
        }
        break;
    case 5:
        if (((EnemyState*)st)->animEventMask != 0)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_eggsnatch);
        }
        break;
    case 6:
        if (((EnemyState*)st)->animEventMask != 0)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_eggsnatch);
        }
        break;
    case 7:
        if (((EnemyState*)st)->animEventMask != 0)
        {
            Sfx_PlayFromObjectLimited((u32)obj, SFXTRIG_baddie_eggsnatch_movelp, 2);
        }
        flag = 1;
        break;
    case 9:
        if (((EnemyState*)st)->animEventMask != 0)
        {
            Sfx_PlayFromObject((int)obj, SFXTRIG_baddie_blooplaugh2);
        }
        break;
    }
    if (flag != 0)
    {
        if (((EnemyState*)st)->phaseAngle != 0)
        {
            (*gPartfxInterface)->spawnObject(obj, FIRECRAWLER_PARTFX_MOVE_TURN, NULL, 2, -1, NULL);
        }
        else
        {
            (*gPartfxInterface)->spawnObject(obj, FIRECRAWLER_PARTFX_MOVE_STRAIGHT, NULL, 2, -1, NULL);
        }
    }
}

void snowworm_update(GameObject* obj, u8* state)
{
    u8* tbl = *(u8**)((char*)gCrawlerReactionTables + ((EnemyState*)state)->phaseAngle * 8);
    int i;

    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumePriority = 10;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumeId = 1;
    if (obj->anim.currentMove == 0)
    {
        obj->anim.resetHitboxFlags =
            obj->anim.resetHitboxFlags | INTERACT_FLAG_DISABLED;
        ObjHits_DisableObject(obj);
    }
    else
    {
        obj->anim.resetHitboxFlags =
            obj->anim.resetHitboxFlags & ~INTERACT_FLAG_DISABLED;
        ObjHits_EnableObject(obj);
    }

    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_JUST_TRIGGERED) != 0 &&
        ((EnemyState*)state)->userData1 <= 1)
    {
        if (((EnemyState*)state)->phaseAngle != 0 || (int)randomGetRange(0, 0x14) < 10)
        {
            ((EnemyState*)state)->userData1 = 1;
        }
        else
        {
            ((EnemyState*)state)->userData1 = 7;
        }
        ((EnemyState*)state)->controlFlags |= (u64)BADDIE_CONTROL_SEQUENCE_DRIVEN;
    }

    if ((((EnemyState*)state)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        *(char*)&((EnemyState*)state)->userData1 += 1;
        if (((EnemyState*)state)->userData1 > gSnowwormSeqIndexMax[((EnemyState*)state)->phaseAngle])
        {
            ((EnemyState*)state)->userData1 = gSnowwormSeqIndexReset[((EnemyState*)state)->phaseAngle];
        }
        if (((EnemyState*)state)->turnOctant < 4)
        {
            i = ((EnemyState*)state)->userData1 * 0xc;
            baddieSetMove(obj, (int)state, (tbl + i)[8], *(f32*)((int)tbl + i), 0, 0);
        }
        else
        {
            i = ((EnemyState*)state)->userData1 * 0xc;
            baddieSetMove(obj, (int)state, (tbl + i)[9], *(f32*)((int)tbl + i), 0, 0);
        }
        if (obj->anim.currentMove == 9)
        {
            snowworm_spawnProjectile(obj);
        }
        else if (obj->anim.currentMove == 1)
        {
            int r = randomGetRange(0, ((EnemyState*)state)->userData2);
            s16 a = randomGetRange(-0x8000, 0x7fff);
            f32 angle = (gCrawlerPi * a) / gCrawlerHalfCircleBams;
            obj->anim.localPosX =
                r * mathSinf(angle) + ((ObjPlacement*)obj->anim.placementData)->posX;
            obj->anim.localPosZ =
                r * mathCosf(angle) + ((ObjPlacement*)obj->anim.placementData)->posZ;
            baddieTurnTowardPoint(obj, (int)state, ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX,
                        ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ, 1, 0);
        }
    }

    baddieTurnTowardPoint(obj, (int)state, ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosX,
                ((GameObject*)((EnemyState*)state)->trackedObj)->anim.localPosZ,
                gSnowwormTurnRates[((EnemyState*)state)->phaseAngle], 0);
    crawler_playReactionEffects(obj, (int*)state);
}

void snowworm_applyReactionState(GameObject* obj, int* st)
{
    u8* t1 = *(u8**)((char*)gCrawlerReactionTables + ((EnemyState*)st)->phaseAngle * 8);
    *((u8*)obj + 0xaf) = (u8)(*((u8*)obj + 0xaf) | 0x8);
    if ((((EnemyState*)st)->controlFlags & BADDIE_CONTROL_SEQUENCE_DRIVEN) != 0)
    {
        s16 a = obj->anim.currentMove;
        if (a == 7)
        {
            ((EnemyState*)st)->userData1 = 1;
        }
        else if (a != 0)
        {
            ((EnemyState*)st)->userData1 = 0;
        }
        {
            u8* bbase = t1;
            f32* fbase = (f32*)t1;
            u32 idx2 = ((EnemyState*)st)->userData1;
            u32 off = idx2 * 0xc;
            baddieSetMove(obj, (int)st, bbase[off + 8], *(f32*)((char*)fbase + off), 0, 0);
        }
    }
    crawler_playReactionEffects(obj, st);
}

void snowworm_init(GameObject* obj, int* st)
{
    ((EnemyState*)st)->sightRange = 60.0f;
    ((EnemyState*)st)->userData2 = ((EnemyState*)st)->aggroRange;
    ((EnemyState*)st)->aggroRange = 160.0f;
    ((EnemyState*)st)->flags2E4 = 0x42003;
    ((EnemyState*)st)->animPlaySpeed = 0.01f;
    ((EnemyState*)st)->gravity = 0.006f;
    ((EnemyState*)st)->drag = 0.95f;
    *((u8*)st + 0x320) = 0;
    {
        f32 d = 1.0f;
        ((EnemyState*)st)->moveSpeedScale0 = d;
        *((u8*)st + 0x321) = 0xa;
        ((EnemyState*)st)->moveSpeedScale1 = d;
        *((u8*)st + 0x322) = 7;
        ((EnemyState*)st)->moveSpeedScale2 = d;
    }
    ((EnemyState*)st)->userData1 = 1;
    ((EnemyState*)st)->phaseAngle = (u16)(obj->anim.romDefNo == SNOWWORM_SEQID_BABY);
}

void whirlpool_updateWhileFrozen(int wpad0, u8* wpad1, int wpad2, int wpad3, int wpad4, int wpad5, Vec* wpad6,
                                 int wpad7)
{
}

#define ICEBADDIE_OBJGROUP_SECONDARY 80
#define ICEBADDIE_HIT_VOLUME_SLOT    10

void iceBaddie_enterWhirlpoolGroup(GameObject* obj, EnemyState* state)
{
    ObjHitsPriorityState* hitState;

    if (state->userData2 == 0)
    {
        ObjGroup_AddObject((int)obj, ICEBADDIE_OBJGROUP_SECONDARY);
        state->userData2 = 1;
    }
    ObjHits_SetHitVolumeSlot(&obj->anim, ICEBADDIE_HIT_VOLUME_SLOT, 1, 0);
    hitState = (ObjHitsPriorityState*)(obj)->anim.hitReactState;
    hitState->suppressOutgoingHits = 0;
    (obj)->anim.rotX -= 256;
}

void iceBaddie_leaveWhirlpoolGroup(GameObject* obj, EnemyState* state)
{
    if (state->userData2 != 0)
    {
        ObjGroup_RemoveObject((int)obj, ICEBADDIE_OBJGROUP_SECONDARY);
        state->userData2 = 0;
    }
    *(u16*)obj = (float)(int)(obj)->anim.rotX - 256.0f * timeDelta;
}

void baddie_initWhirlpoolState(int* obj, EnemyState* state)
{
    f32 fz;
    state->sightRange = 60.0f;
    *(char*)&state->userData2 = state->aggroRange;
    state->aggroRange = 160.0f;
    state->flags2E4 = 0x42001;
    state->animPlaySpeed = 0.01f;
    state->gravity = 0.006f;
    state->drag = 0.95f;
    state->moveId0 = 0;
    fz = 1.0f;
    state->moveSpeedScale0 = fz;
    state->moveId1 = 5;
    state->moveSpeedScale1 = fz;
    state->moveId2 = 7;
    state->moveSpeedScale2 = fz;
    state->userData1 = 1;
    state->userData2 = 0;
    ObjModel_SetRenderCallback((u8*)Obj_GetActiveModel((GameObject*)obj), renderWhirlpool);
}

typedef struct IceBaddieControl {
    f32 hitTimer;               /* 0x00 */
    s16 attackPatternIndex;     /* 0x04 */
    s16 consecutiveHitCount;    /* 0x06 */
    f32 projectileTransform[6]; /* 0x08 */
    f32 particlePositionX;      /* 0x20 */
    f32 particlePositionY;      /* 0x24 */
    f32 fxScale;                /* 0x28 */
    f32 effectPosition[3];      /* 0x2C */
    f32 projectileVelocity[3];  /* 0x38 */
    u8 effectFlags;             /* 0x44 */
    u8 pad45;                   /* 0x45 */
    u16 ambientSfxTimer;        /* 0x46 */
} IceBaddieControl;

typedef struct IceBallSetup {
    ObjPlacement base; /* 0x00 */
    u8 pad18[0x1e - 0x18];
    s16 gameBit;          /* 0x1E */
    s16 secondaryGameBit; /* 0x20 */
    u8 pad22[0x24 - 0x22];
} IceBallSetup;

STATIC_ASSERT(offsetof(IceBaddieControl, attackPatternIndex) == 0x4);
STATIC_ASSERT(offsetof(IceBaddieControl, particlePositionX) == 0x20);
STATIC_ASSERT(offsetof(IceBaddieControl, effectPosition) == 0x2C);
STATIC_ASSERT(offsetof(IceBaddieControl, projectileVelocity) == 0x38);
STATIC_ASSERT(offsetof(IceBaddieControl, effectFlags) == 0x44);
STATIC_ASSERT(sizeof(IceBaddieControl) == 0x48);
STATIC_ASSERT(offsetof(IceBallSetup, gameBit) == 0x1E);
STATIC_ASSERT(offsetof(IceBallSetup, secondaryGameBit) == 0x20);
STATIC_ASSERT(sizeof(IceBallSetup) == 0x24);

#define ICEBADDIE_OBJGROUP 3

#define ICEBADDIE_FX_SPAWN_ICEBALL 0x01 /* fire the armed ice-ball projectile */
#define ICEBADDIE_FX_ARM_ICEBALL   0x02 /* stash spawn transform, then request SPAWN_ICEBALL */
#define ICEBADDIE_FX_BURST         0x04 /* 4x contact particle (obj 0x56) */
#define ICEBADDIE_FX_PUFF          0x08 /* one puff particle (obj 0x57) */
#define ICEBADDIE_FX_IMPACT        0x10 /* camera shake + 0x28x particle 0x57 */
#define ICEBADDIE_FX_LANDING       0x20 /* bigger shake + 0x57 burst + 0x58 debris (anim event 0x200) */

#define ICEBADDIE_CHILD_OBJ_ICEBALL 100

#define ICEBADDIE_PARTICLE_CONTACT 0x56 /* 4x contact particle */
#define ICEBADDIE_PARTICLE_PUFF    0x57 /* puff / impact burst particle */
#define ICEBADDIE_PARTICLE_DEBRIS  0x58 /* landing debris particle */

u8 gIceBaddieA06MoveVariant;
u8 gIceBaddieA05MoveVariant;

IceBaddieStateHandler gIceBaddieStateHandlersB[8];
IceBaddieStateHandler gIceBaddieStateHandlersA[14];

static inline void icebaddie_clearStateHandlersB(void)
{
    int i;
    for (i = 0; i < 8; i++)
        gIceBaddieStateHandlersB[i] = NULL;
}

s16 gIceBaddieAttackMoves[8] = {5, 6, 8, 6, 5, 8, 6, 0};
s16 gIceBaddieAttackMovesAlt[8] = {8, 6, 9, 8, 6, 9, 9, 0};
int gIceBaddieHitReactionMoves[30] = {
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 12,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
};
u8 gIceBaddieHitReactionDamage[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
};
u8 gIceBaddieParticleArgsTable[16] = {
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xC0, 0x96, 0x5A, 0x5A, 0x64, 0xFF, 0x5A, 0x00, 0x00,
};
u8 gIceBaddiePaletteIndexTable[32] = {
    0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void iceBaddie_spawnIceBall(GameObject* obj, IceBaddieControl* control);
void iceBaddie_updateControlEffects(GameObject* obj, GroundBaddieState* state);
void iceBaddie_tryAcquireTarget(GameObject* obj, GroundBaddieState* objectState, GroundBaddieState* state);
void iceBaddie_updateTargetMotion(GameObject* obj, GroundBaddieState* objectState, GroundBaddieState* state);
void iceBaddie_updateTargetCollision(GameObject* obj, int stateAddress, GroundBaddieState* state);

int iceBaddie_stateHandlerB07(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;

    if ((s8)state->baddie.moveJustStartedB != 0) {
        if ((s32)state->baddie.targetDistance > 0x37) {
            if ((objectState->configFlags & 2) == 0) {
                (*gPlayerInterface)->setState(obj, state, 7);
            } else {
                IceBaddieControl* control = (IceBaddieControl*)objectState->control;
                if ((objectState->configFlags & 0x10) != 0) {
                    (*gPlayerInterface)->setState(obj, state, gIceBaddieAttackMovesAlt[control->attackPatternIndex++]);
                } else {
                    (*gPlayerInterface)->setState(obj, state, gIceBaddieAttackMoves[control->attackPatternIndex++]);
                }
                if (control->attackPatternIndex >= 7) {
                    control->attackPatternIndex = 0;
                }
            }
        } else {
            if (state->baddie.controlMode == 6) {
                (*gPlayerInterface)->setState(obj, state, 5);
            } else {
                (*gPlayerInterface)->setState(obj, state, 6);
            }
        }
    } else if (state->baddie.moveDone != 0) {
        if (((*gBaddieControlInterface)->getClearDirectionMask(obj, state, 75.0f) & 1) == 0) {
            return 5;
        }
        if ((*gBaddieControlInterface)->shouldDropTarget(obj, state, objectState->aggroRange, 1) != 0) {
            return 5;
        }
        if ((s32)state->baddie.targetDistance > 0x37) {
            if ((objectState->configFlags & 2) == 0) {
                (*gPlayerInterface)->setState(obj, state, 7);
            } else {
                IceBaddieControl* control = (IceBaddieControl*)objectState->control;
                if ((objectState->configFlags & 0x10) != 0) {
                    (*gPlayerInterface)->setState(obj, state, gIceBaddieAttackMovesAlt[control->attackPatternIndex++]);
                } else {
                    (*gPlayerInterface)->setState(obj, state, gIceBaddieAttackMoves[control->attackPatternIndex++]);
                }
                if (control->attackPatternIndex >= 7) {
                    control->attackPatternIndex = 0;
                }
            }
        } else {
            if (state->baddie.controlMode == 6) {
                (*gPlayerInterface)->setState(obj, state, 5);
            } else {
                (*gPlayerInterface)->setState(obj, state, 6);
            }
        }
    } else if (state->baddie.controlMode == 7 && (s32)state->baddie.targetDistance < 0x37) {
        if (state->baddie.controlMode == 6) {
            (*gPlayerInterface)->setState(obj, state, 5);
        } else {
            (*gPlayerInterface)->setState(obj, state, 6);
        }
    }
    return 0;
}

int iceBaddie_stateHandlerB06(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;
    RouteNav* route;
    f32 neutralBlend;

    if (state->baddie.moveDone != 0 &&
        (((*gBaddieControlInterface)->getClearDirectionMask(obj, state, 75.0f) & 1) == 0)) {
        return 5;
    }
    if ((s8)state->baddie.moveJustStartedB != 0) {
        (*gPlayerInterface)->setState(obj, state, 0xb);
    } else if (objectState->targetState == 3) {
        (*gPlayerInterface)->setState(obj, state, 4);
    } else if (objectState->targetState == 4) {
        if (state->baddie.targetDistance < 110.0f && state->baddie.moveDone != 0) {
            if (objectState->aggression > 50) {
                (*gPlayerInterface)->setState(obj, state, 0);
            } else {
                (*gPlayerInterface)->setState(obj, state, 1);
            }
        }
    } else if (objectState->targetState == 1) {
        return 8;
    }
    route = &objectState->routeNav;
    neutralBlend = 0.0f;
    state->baddie.moveInputX = neutralBlend;
    state->baddie.moveInputZ = neutralBlend;
    memcpy(route, &obj->anim.localPosX, 0xc);
    memcpy((void*)objectState->routeNav.curPos, (void*)&((GameObject*)state->baddie.targetObj)->anim.localPosX, 0xc);
    voxmaps_updateRoutePath(&objectState->routeNav, &objectState->routeState);
    if (route->flag25 == 0) {
        (*gPlayerInterface)->moveTowardPoint(obj, state, route->tgtPos[0], route->tgtPos[2], 0.0f, 0.0f, 60.0f);
    } else {
        (*gPlayerInterface)->moveTowardPoint(obj, state, route->tgtPos[0], route->tgtPos[2], 15.0f, 30.0f, 60.0f);
    }
    if (state->baddie.stateTimer > 0x78 &&
        (*gBaddieControlInterface)->shouldDropTarget(obj, state, objectState->aggroRange, 1) != 0) {
        return 5;
    }
    return 0;
}

int iceBaddie_stateHandlerB05(GameObject* obj, GroundBaddieState* state) {
    if ((s8)state->baddie.moveJustStartedB != 0) {
        (*gPlayerInterface)->setState(obj, state, 3);
    }
    if (state->baddie.moveDone != 0) {
        if (state->baddie.controlMode == 3) {
            (*gPlayerInterface)->setState(obj, state, 0);
        } else {
            return 8;
        }
    }
    return 0;
}

int iceBaddie_stateHandlerB04(GameObject* obj, GroundBaddieState* state) {
    if ((s8)state->baddie.moveJustStartedB != 0) {
        (*gPlayerInterface)->setState(obj, state, 2);
    }
    return 0;
}

int iceBaddie_stateHandlerB03(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState;

    if ((s8)state->baddie.moveJustStartedB != 0) {
        objectState = obj->extra;
        objectState->subMode = 0;
        mainSetBits((s32)objectState->gameBitB, 0);
        mainSetBits((s32)objectState->gameBitA, 1);
    }
    return 0;
}

int iceBaddie_stateHandlerB02(GameObject* obj, GroundBaddieState* state) {
    if ((s8)state->baddie.moveJustStartedB != 0) {
        (*gPlayerInterface)->setState(obj, state, 0xd);
        state->baddie.targetObj = NULL;
        state->baddie.physicsActive = 0;
        state->baddie.hasTarget = 0;
        ObjHits_DisableObject(obj);
        obj->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
    } else if (state->baddie.moveDone != 0) {
        ObjMsg_SendToObjects(0, 3, obj, 0xe0000, (int)obj);
        if (obj->anim.placementData == NULL) {
            Obj_FreeObject(obj);
            return 0;
        }
        return 4;
    }
    return 0;
}

int iceBaddie_stateHandlerB01(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;

    if (state->baddie.hitPoints < 1) {
        return 3;
    }
    if (state->baddie.moveDone != 0) {
        if (state->baddie.controlMode == 12) {
            if (objectState->aggression > 50) {
                (*gPlayerInterface)->setState(obj, state, 0);
            } else {
                (*gPlayerInterface)->setState(obj, state, 1);
            }
        } else {
            return 8;
        }
    }
    return 0;
}

int iceBaddie_checkTargetState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;
    f32 neutralBlend;

    if (state->baddie.targetObj != NULL) {
        if ((s32)(s8)state->baddie.moveJustStartedB != 0) {
            neutralBlend = 0.0f;
            state->baddie.animSpeedB = neutralBlend;
            state->baddie.animSpeedA = neutralBlend;
            if ((u32)objectState->aggression > 50) {
                if (state->baddie.targetDistance < 0.5f * (f32)(u32)objectState->aggroRange ||
                    (objectState->configFlags & 0x2) != 0) {
                    (*gPlayerInterface)->setState(obj, state, 0);
                } else {
                    (*gPlayerInterface)->setState(obj, state, 1);
                }
            } else {
                (*gPlayerInterface)->setState(obj, state, 1);
            }
        }

        if ((s32)state->baddie.moveDone != 0) {
            (*gPlayerInterface)->rotateTowardTarget(obj, state, timeDelta, 4);
            if (((*gBaddieControlInterface)->getClearDirectionMask(obj, state, 75.0f) & 1) == 0) {
                return 5;
            }

            if (state->baddie.targetDistance < 0.5f * (f32)(u32)objectState->aggroRange ||
                (objectState->configFlags & 0x2) != 0) {
                return 8;
            }
            return 7;
        }
    }
    return 0;
}

int iceBaddie_updateLandingState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;
    GameObject* player;
    f32 noBlend;

    state->baddie.stateTag = 3;
    state->baddie.moveSpeed = 0.008f;
    noBlend = 0.0f;
    state->baddie.animSpeedA = noBlend;
    state->baddie.animSpeedB = noBlend;
    if (state->baddie.moveJustStartedA != 0) {
        ObjAnim_SetCurrentMove((int)obj, 1, noBlend, 0);
        state->baddie.moveDone = 0;
    }
    if ((state->baddie.moveEventFlags & 1) == 0) {
        player = Obj_GetPlayerObject();
        if (player->anim.romDefNo != 0) {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_stftest122_1f2);
        } else {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_swd);
        }
        Sfx_PlayFromObject((u32)obj, SFXTRIG_en_rfall5_c);
        Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_26f);
        state->baddie.moveEventFlags |= 1;
    }
    if ((state->baddie.moveEventFlags & 2) == 0 && obj->anim.currentMoveProgress > 0.3f) {
        Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_iceywindlp16_233);
        state->baddie.moveEventFlags |= 2;
        (*gBaddieControlInterface)->spawnChild(obj, objectState->triggerId, -1, 0);
    }
    return 0;
}

int iceBaddie_updateContactHitState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;
    IceBaddieControl* control;
    f32 noBlend;

    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumePriority = 10;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumeId = 1;
    ObjHits_RegisterActiveHitVolumeObject(obj);
    if (objectState->aggression > 0x32) {
        if (state->baddie.moveJustStartedA != 0) {
            ObjAnim_SetCurrentMove((int)obj, 4, 0.0f, 0);
            state->baddie.moveDone = 0;
        }
    } else if (state->baddie.moveJustStartedA != 0) {
        ObjAnim_SetCurrentMove((int)obj, 0xe, 0.0f, 0);
        state->baddie.moveDone = 0;
    }
    state->baddie.stateTag = 3;
    state->baddie.moveSpeed = 0.008f;
    control = (IceBaddieControl*)objectState->control;
    control->effectFlags |= (ICEBADDIE_FX_BURST | ICEBADDIE_FX_PUFF);
    noBlend = 0.0f;
    state->baddie.animSpeedA = noBlend;
    state->baddie.animSpeedB = noBlend;
    if ((objectState->configFlags & 2) == 0) {
        state->baddie.animSpeedA = -1.0f + obj->anim.currentMoveProgress;
    }
    return 0;
}

int iceBaddie_stateHandlerA0B(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;
    IceBaddieControl* control;

    if (state->baddie.moveJustStartedA != 0) {
        obj->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
        if (state->baddie.moveJustStartedA != 0) {
            ObjAnim_SetCurrentMove((int)obj, 2, 0.0f, 0);
            state->baddie.moveDone = 0;
        }
        objectState->targetState = 2;
        state->baddie.stateTag = 1;
        state->baddie.moveSpeed = 0.015f;
    } else {
        if (state->baddie.moveDone != 0) {
            objectState->targetState = 3;
        }
    }
    control = (IceBaddieControl*)objectState->control;
    control->effectFlags |= ICEBADDIE_FX_BURST;
    if ((s32)(state->baddie.eventFlags & BADDIE_EVENT_LANDING) != 0) {
        state->baddie.eventFlags &= ~BADDIE_EVENT_LANDING;
        control->effectFlags |= ICEBADDIE_FX_IMPACT;
    }
    control->effectFlags |= (ICEBADDIE_FX_BURST | ICEBADDIE_FX_PUFF);
    state->baddie.animSpeedA = obj->anim.currentMoveProgress;
    return 0;
}

int iceBaddie_updateDropState(GameObject* obj, GroundBaddieState* state) {
    IceBaddieControl* control = (IceBaddieControl*)((GroundBaddieState*)obj->extra)->control;
    GameObject* player;

    control->effectFlags |= ICEBADDIE_FX_BURST;
    if (state->baddie.moveJustStartedA != 0) {
        ObjAnim_SetCurrentMove((int)obj, 0, 0.0f, 0);
        state->baddie.moveDone = 0;
    }
    if (state->baddie.moveJustStartedA != 0) {
        Obj_GetPlayerObject();
        player = Obj_GetPlayerObject();
        if (player->anim.romDefNo != 0) {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_stftest122_1f2);
        } else {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_swd);
        }
        Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_26e);
    }
    state->baddie.stateTag = 3;
    state->baddie.moveSpeed = 0.015f;
    state->baddie.animSpeedA = 0.0f;
    return 0;
}

int iceBaddie_updateCommDownState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;
    IceBaddieControl* control = (IceBaddieControl*)objectState->control;

    control->effectFlags |= ICEBADDIE_FX_BURST;
    state->baddie.moveSpeed = 0.01f;
    if (state->baddie.moveJustStartedA != 0) {
        ObjAnim_SetCurrentMove((int)obj, 10, 0.0f, 0);
        state->baddie.moveDone = 0;
    }
    state->baddie.stateTag = 1;
    if ((state->baddie.eventFlags & 1) != 0) {
        control = (IceBaddieControl*)objectState->control;
        state->baddie.eventFlags &= ~BADDIE_EVENT_FOOTSTEP;
        control->effectFlags |= ICEBADDIE_FX_ARM_ICEBALL;
        Sfx_PlayFromObject((u32)obj, SFXTRIG_wp_dsmk2_c_cf);
    }
    (*gPlayerInterface)->rotateTowardTarget(obj, state, timeDelta, 4);
    return 0;
}

int iceBaddie_updateControlMove5State(GameObject* obj, GroundBaddieState* state) {
    IceBaddieControl* control = (IceBaddieControl*)((GroundBaddieState*)obj->extra)->control;
    control->effectFlags |= ICEBADDIE_FX_BURST;
    state->baddie.moveSpeed = 0.01f;
    if (state->baddie.moveJustStartedA != 0) {
        ObjAnim_SetCurrentMove((int)obj, 5, 0.0f, 0);
        state->baddie.moveDone = 0;
    }
    state->baddie.stateTag = 1;
    (*gPlayerInterface)->rotateTowardTarget(obj, state, timeDelta, 4);
    return 0;
}

int iceBaddie_updateHeightBlendState(GameObject* obj, GroundBaddieState* state) {
    IceBaddieControl* control = (IceBaddieControl*)((GroundBaddieState*)obj->extra)->control;
    f32 height;

    control->effectFlags |= (ICEBADDIE_FX_BURST | ICEBADDIE_FX_PUFF);
    if (state->baddie.moveJustStartedA != 0) {
        if (state->baddie.moveJustStartedA != 0) {
            ObjAnim_SetCurrentMove((int)obj, 0xf, 0.0f, 0);
            state->baddie.moveDone = 0;
        }
        state->baddie.stateTag = 1;
    }
    state->baddie.moveSpeed = state->baddie.targetDistance / 5000.0f;
    if (state->baddie.moveSpeed > 0.02f) {
        state->baddie.moveSpeed = 0.02f;
    } else if (state->baddie.moveSpeed < 0.01f) {
        state->baddie.moveSpeed = 0.01f;
    }
    height = obj->anim.currentMoveProgress;
    if (height < 0.5f) {
        state->baddie.animSpeedA = 4.0f * height;
    } else {
        state->baddie.animSpeedA = 4.0f * (1.0f - height);
    }
    (*gPlayerInterface)->rotateTowardTarget(obj, state, timeDelta, 4);
    return 0;
}

int iceBaddie_stateHandlerA06(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;
    int moveChoice;

    ((IceBaddieControl*)objectState->control)->effectFlags |= ICEBADDIE_FX_BURST;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumePriority = 10;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumeId = 1;
    ObjHits_RegisterActiveHitVolumeObject(obj);
    if (state->baddie.moveJustStartedA != 0) {
        gIceBaddieA06MoveVariant = randomGetRange(0, 2);
        moveChoice = randomGetRange(0, 1);
        if (moveChoice != 0) {
            if (state->baddie.moveJustStartedA != 0) {
                ObjAnim_SetCurrentMove((int)obj, 7, 0.0f, 0);
                state->baddie.moveDone = 0;
            }
        } else {
            if (state->baddie.moveJustStartedA != 0) {
                ObjAnim_SetCurrentMove((int)obj, 3, 0.0f, 0);
                state->baddie.moveDone = 0;
            }
        }
        state->baddie.stateTag = 1;
        state->baddie.moveSpeed = 0.005f + objectState->aggression / 20000.0f;
    }
    if (objectState->aggression > 50 && (objectState->configFlags & 2) == 0) {
        if (state->baddie.targetDistance > 55.0f && state->baddie.moveDone == 0) {
            state->baddie.animSpeedA = state->baddie.targetDistance / 55.0f - 1.0f;
            state->baddie.animSpeedA = state->baddie.animSpeedA * ((f32)objectState->aggression / 50.0f);
        } else {
            state->baddie.animSpeedA = 0.0f;
        }
    } else {
        state->baddie.animSpeedA = 0.0f;
    }
    (*gPlayerInterface)->rotateTowardTarget(obj, state, timeDelta, 4);
    return 0;
}

int iceBaddie_stateHandlerA05(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;
    int moveChoice;

    ((IceBaddieControl*)objectState->control)->effectFlags |= ICEBADDIE_FX_BURST;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumePriority = 10;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumeId = 1;
    ObjHits_RegisterActiveHitVolumeObject(obj);
    if (state->baddie.moveJustStartedA != 0) {
        moveChoice = randomGetRange(0, 1);
        if (moveChoice != 0) {
            gIceBaddieA05MoveVariant = randomGetRange(0, 2);
            if (state->baddie.moveJustStartedA != 0) {
                ObjAnim_SetCurrentMove((int)obj, 6, 0.0f, 0);
                state->baddie.moveDone = 0;
            }
        } else {
            gIceBaddieA05MoveVariant = 3;
            if (state->baddie.moveJustStartedA != 0) {
                ObjAnim_SetCurrentMove((int)obj, 10, 0.0f, 0);
                state->baddie.moveDone = 0;
            }
        }
        state->baddie.stateTag = 1;
        state->baddie.moveSpeed = 0.005f + objectState->aggression / 20000.0f;
    }
    if (objectState->aggression > 50 && (objectState->configFlags & 2) == 0) {
        if (state->baddie.targetDistance > 55.0f && state->baddie.moveDone == 0) {
            state->baddie.animSpeedA = state->baddie.targetDistance / 55.0f - 1.0f;
            state->baddie.animSpeedA = state->baddie.animSpeedA * ((f32)objectState->aggression / 50.0f);
        } else {
            state->baddie.animSpeedA = 0.0f;
        }
    } else {
        state->baddie.animSpeedA = 0.0f;
    }
    (*gPlayerInterface)->rotateTowardTarget(obj, state, timeDelta, 4);
    return 0;
}

int iceBaddie_updateSpinState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;
    IceBaddieControl* control;

    if (state->baddie.moveJustStartedA != 0) {
        ObjAnim_SetCurrentMove((int)obj, 9, 0.0f, 0);
        state->baddie.moveDone = 0;
    }
    control = (IceBaddieControl*)objectState->control;
    control->effectFlags |= (ICEBADDIE_FX_BURST | ICEBADDIE_FX_PUFF);
    if (state->baddie.moveJustStartedA != 0) {
        obj->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
        objectState->targetState = 4;
    }
    obj->anim.rotX = (s16)(182.0f * (((f32)state->baddie.turnRate * timeDelta) / 12.0f) + (f32) * (s16*)obj);
    state->baddie.moveSpeed = 0.01f;
    state->baddie.animSpeedA = 1.0f;
    return 0;
}

int iceBaddie_updateImpactHitState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;
    IceBaddieControl* control = (IceBaddieControl*)objectState->control;

    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumePriority = 10;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumeId = 1;
    ObjHits_RegisterActiveHitVolumeObject(obj);
    if (state->baddie.moveJustStartedA != 0) {
        state->baddie.moveDone = 0;
    }
    if (state->baddie.moveJustStartedA != 0) {
        ObjAnim_SetCurrentMove((int)obj, 4, 0.0f, 0);
        state->baddie.moveDone = 0;
    }
    state->baddie.stateTag = 3;
    state->baddie.moveSpeed = 0.008f;
    if ((s32)(state->baddie.eventFlags & BADDIE_EVENT_LANDING) != 0) {
        state->baddie.eventFlags &= ~BADDIE_EVENT_LANDING;
        control->effectFlags |= ICEBADDIE_FX_IMPACT;
    }
    control->effectFlags |= (ICEBADDIE_FX_BURST | ICEBADDIE_FX_PUFF);
    return 0;
}

int iceBaddie_updateHideResetState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState = obj->extra;
    ObjHitsPriorityState* hitState;

    if (state->baddie.prevControlMode != 4 && state->baddie.moveJustStartedA != 0) {
        ObjAnim_SetCurrentMove((int)obj, 0xe, 0.0f, 0);
        state->baddie.moveDone = 0;
    }
    ((IceBaddieControl*)objectState->control)->effectFlags |= (ICEBADDIE_FX_BURST | ICEBADDIE_FX_PUFF);
    if (state->baddie.moveJustStartedA != 0) {
        hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
        hitState->flags &= ~1;
        state->baddie.moveSpeed = 0.01f;
        state->baddie.animSpeedA = 0.0f;
    }
    if (state->baddie.moveDone != 0) {
        mainSetBits((s32)objectState->gameBitB, 0);
        ObjAnim_SetCurrentMove((int)obj, 8, 0.0f, 0);
        state->baddie.targetObj = NULL;
        state->baddie.physicsActive = 0;
        state->baddie.hasTarget = 0;
        objectState->targetState = 0;
        obj->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
    }
    return 0;
}

int iceBaddie_updateOpenState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState;
    IceBaddieControl* control;
    ObjHitsPriorityState* hitState;

    objectState = obj->extra;
    control = (IceBaddieControl*)objectState->control;
    hitState = (ObjHitsPriorityState*)obj->anim.hitReactState;
    hitState->flags |= 1;
    state->baddie.physicsActive = 1;
    if (state->baddie.moveJustStartedA != 0) {
        ObjAnim_SetCurrentMove((int)obj, 11, 0.0f, 0);
        state->baddie.moveDone = 0;
    }
    if (state->baddie.moveJustStartedA != 0) {
        mainSetBits(objectState->gameBitB, 1);
        obj->anim.resetHitboxFlags &= ~INTERACT_FLAG_DISABLED;
        obj->anim.alpha = 0xff;
        state->baddie.stateTag = 1;
        state->baddie.moveSpeed = 0.012f + (f32)(u32)objectState->aggression / 10000.0f;
    }
    if (state->baddie.moveDone != 0) {
        objectState->targetState = 1;
    }
    {
        int eventFlags = state->baddie.eventFlags;
        if ((eventFlags & BADDIE_EVENT_LANDING) != 0) {
            state->baddie.eventFlags = eventFlags & ~BADDIE_EVENT_LANDING;
            control->effectFlags |= ICEBADDIE_FX_LANDING;
        }
    }
    control->effectFlags |= ICEBADDIE_FX_BURST;
    if (obj->anim.currentMoveProgress < 0.4f) {
        control->effectFlags |= ICEBADDIE_FX_PUFF;
    }
    (*gPlayerInterface)->rotateTowardTarget(obj, state, timeDelta, 4);
    return 0;
}

int iceBaddie_updateOpenHitState(GameObject* obj, GroundBaddieState* state) {
    GroundBaddieState* objectState;
    IceBaddieControl* control;

    objectState = obj->extra;
    control = (IceBaddieControl*)objectState->control;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->flags |= OBJHITS_PRIORITY_STATE_ENABLED;
    state->baddie.physicsActive = 1;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumePriority = 9;
    ((ObjHitsPriorityState*)obj->anim.hitReactState)->hitVolumeId = 1;
    ObjHits_RegisterActiveHitVolumeObject(obj);
    if (state->baddie.moveJustStartedA != 0) {
        ObjAnim_SetCurrentMove((int)obj, 8, 0.0f, 0);
        state->baddie.moveDone = 0;
    }
    if (state->baddie.moveJustStartedA != 0) {
        mainSetBits(objectState->gameBitB, 1);
        obj->anim.resetHitboxFlags &= ~INTERACT_FLAG_DISABLED;
        obj->anim.alpha = 0xff;
        state->baddie.stateTag = 1;
        state->baddie.moveSpeed = 0.0025f + (f32)(u32)objectState->aggression / 50000.0f;
    }
    if (state->baddie.moveDone != 0) {
        objectState->targetState = 1;
    }
    {
        int eventFlags = state->baddie.eventFlags;
        if ((eventFlags & BADDIE_EVENT_LANDING) != 0) {
            state->baddie.eventFlags = eventFlags & ~BADDIE_EVENT_LANDING;
            control->effectFlags |= ICEBADDIE_FX_LANDING;
        }
    }
    control->effectFlags |= ICEBADDIE_FX_BURST;
    if (obj->anim.currentMoveProgress < 0.4f) {
        control->effectFlags |= ICEBADDIE_FX_PUFF;
    }
    (*gPlayerInterface)->rotateTowardTarget(obj, state, timeDelta, 4);
    return 0;
}

void iceBaddie_spawnIceBall(GameObject* obj, IceBaddieControl* control) {
    IceBallSetup* setup;
    GameObject* projectile;
    if ((u8)Obj_IsLoadingLocked() != 0) {
        setup = (IceBallSetup*)Obj_AllocObjectSetup(36, ICEBADDIE_CHILD_OBJ_ICEBALL);
        setup->base.posX = control->projectileTransform[3];
        setup->base.posY = control->projectileTransform[4];
        setup->base.posZ = control->projectileTransform[5];
        setup->base.color[0] = 1;
        setup->base.color[1] = 1;
        setup->base.color[2] = 255;
        setup->base.color[3] = 255;
        setup->gameBit = -1;
        setup->secondaryGameBit = -1;
        projectile = Obj_SetupObject(&setup->base, 5, -1, -1, NULL);
        if (projectile != NULL) {
            projectile->anim.velocityX = control->projectileVelocity[0];
            projectile->anim.velocityY = control->projectileVelocity[1];
            projectile->anim.velocityZ = control->projectileVelocity[2];
            projectile->ownerObj = obj;
        }
    }
}

void iceBaddie_updateControlEffects(GameObject* obj, GroundBaddieState* state) {
    int controlAddress = (int)state->control;
    int paletteIndex;
    u8* particleArgs;
    int i;
    f32 shakeScale;
    f32 contactScale;

    if (obj->anim.romDefNo == 99) {
        ((IceBaddieControl*)controlAddress)->fxScale = 1.7f;
        shakeScale = 2.0f;
    } else {
        contactScale = 1.0f;
        ((IceBaddieControl*)controlAddress)->fxScale = contactScale;
        shakeScale = contactScale;
    }
    paletteIndex = 0;
    if ((s8)state->baddie.physicsActive != 0) {
        paletteIndex = gIceBaddiePaletteIndexTable[(s8)state->baddie.paletteSlot];
        if (paletteIndex > 0x1e) {
            paletteIndex = 0;
        }
    }
    particleArgs = &gIceBaddieParticleArgsTable[paletteIndex * 3];
    if ((((IceBaddieControl*)controlAddress)->effectFlags & ICEBADDIE_FX_SPAWN_ICEBALL) != 0) {
        iceBaddie_spawnIceBall(obj, (IceBaddieControl*)controlAddress);
        ((IceBaddieControl*)controlAddress)->effectFlags &= ~ICEBADDIE_FX_SPAWN_ICEBALL;
    }
    if ((((IceBaddieControl*)controlAddress)->effectFlags & ICEBADDIE_FX_BURST) != 0 &&
        (state->configFlags & 0x40) == 0) {
        for (i = 0; i < 4; i++) {
            (*gPartfxInterface)
                ->spawnObject((void*)obj, ICEBADDIE_PARTICLE_CONTACT, (void*)(controlAddress + 0x20), 0x200001, -1,
                              particleArgs);
        }
    }
    if ((((IceBaddieControl*)controlAddress)->effectFlags & ICEBADDIE_FX_PUFF) != 0 &&
        (state->configFlags & 0x40) == 0) {
        (*gPartfxInterface)
            ->spawnObject((void*)obj, ICEBADDIE_PARTICLE_PUFF, (void*)(controlAddress + 0x20), 0x200001, -1,
                          particleArgs);
    }
    if ((((IceBaddieControl*)controlAddress)->effectFlags & ICEBADDIE_FX_IMPACT) != 0) {
        CameraShake_Enable();
        CameraShake_SetOffset(2.0f * shakeScale);
        for (i = 0; i < 0x28; i++) {
            (*gPartfxInterface)
                ->spawnObject((void*)obj, ICEBADDIE_PARTICLE_PUFF, (void*)(controlAddress + 0x20), 0x200001, -1,
                              particleArgs);
        }
    }
    if ((((IceBaddieControl*)controlAddress)->effectFlags & ICEBADDIE_FX_LANDING) != 0) {
        CameraShake_Enable();
        CameraShake_SetOffset(3.0f * shakeScale);
        for (i = 0; i < 0x28; i++) {
            (*gPartfxInterface)
                ->spawnObject((void*)obj, ICEBADDIE_PARTICLE_PUFF, (void*)(controlAddress + 0x20), 0x200001, -1,
                              particleArgs);
        }
        for (i = 0; i < 10; i++) {
            (*gPartfxInterface)
                ->spawnObject((void*)obj, ICEBADDIE_PARTICLE_DEBRIS, (void*)(controlAddress + 0x20), 0x200001, -1,
                              particleArgs);
        }
    }
    ((IceBaddieControl*)controlAddress)->effectFlags = 0;
}

void iceBaddie_updateEffectAnchors(GameObject* obj, GroundBaddieState* state) {
    IceBaddieControl* control = (IceBaddieControl*)state->control;
    f32 transformed[3];
    f32 transformScratch[6];
    f32 pathMtx[16];
    f32 scale;
    f32 minScale;
    f32 angle;

    memcpy(pathMtx, (void*)ObjPath_GetPointModelMtx(obj, 1), 0x40);
    pathMtx[14] = 0.0f;
    pathMtx[13] = 0.0f;
    pathMtx[12] = 0.0f;
    if (obj->anim.romDefNo == 99) {
        minScale = 1.0f;
    } else {
        minScale = 0.3f;
    }
    if (state->baddie.animSpeedA < minScale) {
        scale = minScale;
    } else {
        scale = state->baddie.animSpeedA;
    }
    if (state->baddie.controlMode != 4) {
        ObjPath_GetPointWorldPosition(obj, 2, &control->effectPosition[0], &control->effectPosition[1],
                                      &control->effectPosition[2], 0);
    } else {
        ObjPath_GetPointWorldPosition(obj, 0, &control->effectPosition[0], &control->effectPosition[1],
                                      &control->effectPosition[2], 0);
    }
    control->effectPosition[1] = 8.0f + obj->anim.localPosY;
    angle = (3.1415927f * (f32) * (s16*)obj) / 32768.0f;
    control->effectPosition[0] = control->effectPosition[0] - scale * (10.0f * mathSinf(angle));
    angle = (3.1415927f * (f32) * (s16*)obj) / 32768.0f;
    control->effectPosition[2] = control->effectPosition[2] - scale * (10.0f * mathCosf(angle));
    transformScratch[3] = 0.0f;
    transformScratch[4] = -15.0f;
    transformScratch[5] = -20.0f;
    ObjPath_GetPointWorldPosition(obj, 0, &transformScratch[3], &transformScratch[4], &transformScratch[5], 1);
    if ((control->effectFlags & ICEBADDIE_FX_ARM_ICEBALL) != 0) {
        transformed[0] = -8.0f;
        transformed[1] = 40.0f;
        transformed[2] = -20.0f;
        Matrix_TransformPoint(pathMtx, transformed[0], transformed[1], transformed[2], &transformed[0], &transformed[1],
                              &transformed[2]);
        memcpy(control->projectileVelocity, transformed, sizeof(transformed));
        memcpy(control->projectileTransform, transformScratch, 0x18);
        control->effectFlags |= ICEBADDIE_FX_SPAWN_ICEBALL;
    }
}

void iceBaddie_tryAcquireTarget(GameObject* obj, GroundBaddieState* objectState, GroundBaddieState* state) {
    GameObject* acquired;

    ObjHits_DisableObject(obj);

    if ((objectState->configFlags & 0x4) != 0) {
        acquired = (*gBaddieControlInterface)->findAggroTarget(obj, state, 55.0f, 0x8000);
    } else if ((objectState->configFlags & 0x8) != 0) {
        acquired =
            (*gBaddieControlInterface)->findAggroTarget(obj, state, 0.5f * (f32)(u32)objectState->aggroRange, 0x8000);
    } else {
        acquired = (*gBaddieControlInterface)->findAggroTarget(obj, state, (f32)(u32)objectState->aggroRange, 0x8000);
    }

    if (acquired != 0) {
        (*gPlayerInterface)->rotateTowardTarget(obj, state, timeDelta, 4);
        if (((*gBaddieControlInterface)->getClearDirectionMask(obj, state, 75.0f) & 1) == 0) {
            acquired = 0;
        }
    }

    if (acquired != 0) {
        int physicsActive = -1;
        (*gBaddieControlInterface)
            ->startHitReaction(obj, state, &objectState->routeNav, objectState->gameBitB, NULL, 0, 0, 8,
                               physicsActive);
        state->baddie.targetObj = acquired;
        state->baddie.hasTarget = 0;
        objectState->targetState = 1;
    }
}

void iceBaddie_updateTargetMotion(GameObject* obj, GroundBaddieState* objectState, GroundBaddieState* state) {
    IceBaddieControl* control = (IceBaddieControl*)objectState->control;

    control->ambientSfxTimer += framesThisStep;
    if (control->ambientSfxTimer >= 300) {
        control->ambientSfxTimer = randomGetRange(0, 200);
        if (state->baddie.controlMode == 7 || state->baddie.controlMode == 8) {
            Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_boar1_c_26c);
        }
    }
    if ((objectState->configFlags & 2) != 0) {
        (*gBaddieControlInterface)->updateGravity(obj, state, 0.0f, -1);
    } else {
        (*gBaddieControlInterface)->updateGravity(obj, state, 0.17f, -1);
    }
    objectState->savedPendingParentObj = obj->pendingParentObj;
    obj->pendingParentObj = NULL;
    (*gPlayerInterface)->update(obj, state, timeDelta, timeDelta, gIceBaddieStateHandlersA, gIceBaddieStateHandlersB);
    obj->pendingParentObj = objectState->savedPendingParentObj;
}

void iceBaddie_updateTargetCollision(GameObject* obj, int stateAddress, GroundBaddieState* state) {
    int controlAddress = (int)((GroundBaddieState*)stateAddress)->control;
    GameObject* target;
    int hitInfo[7];
    f32 targetDelta[3];

    Obj_GetPlayerObject();
    target = state->baddie.targetObj;
    if (target != NULL) {
        f32* delta = targetDelta;
        delta[0] = target->anim.worldPosX - obj->anim.worldPosX;
        delta[1] = target->anim.worldPosY - obj->anim.worldPosY;
        delta[2] = target->anim.worldPosZ - obj->anim.worldPosZ;
        state->baddie.targetDistance = sqrtf(delta[2] * delta[2] + (delta[0] * delta[0] + delta[1] * delta[1]));
    }
    if ((((GroundBaddieState*)stateAddress)->configFlags & 0x20) == 0) {
        (*gBaddieControlInterface)
            ->pollCameraTarget(obj, state, &((GroundBaddieState*)stateAddress)->flags400, 2, 3,
                               ((GroundBaddieState*)stateAddress)->soundIdB,
                               ((GroundBaddieState*)stateAddress)->soundIdA);
    }
    (*gBaddieControlInterface)
        ->processMessages(obj, state, (void*)(stateAddress + 0x35c), ((GroundBaddieState*)stateAddress)->gameBitB, NULL,
                          0, 0, 8);
    ((IceBaddieControl*)controlAddress)->hitTimer += timeDelta;
    if (state->baddie.controlMode != 3 &&
        (*gBaddieControlInterface)
                ->updateHitReaction(obj, state, &((GroundBaddieState*)stateAddress)->routeNav,
                                    ((GroundBaddieState*)stateAddress)->gameBitB, gIceBaddieHitReactionMoves,
                                    gIceBaddieHitReactionDamage, 1, hitInfo) != 0) {
        if (((IceBaddieControl*)controlAddress)->hitTimer < 240.0f) {
            ((IceBaddieControl*)controlAddress)->consecutiveHitCount += 1;
        } else {
            ((IceBaddieControl*)controlAddress)->consecutiveHitCount = 0;
        }
        ((IceBaddieControl*)controlAddress)->hitTimer = 0.0f;
        if (state->baddie.hitPoints > 0 && ((IceBaddieControl*)controlAddress)->consecutiveHitCount >= 2) {
            (*gPlayerInterface)->setState(obj, state, 3);
            ((IceBaddieControl*)controlAddress)->consecutiveHitCount = 0;
            state->baddie.substate = 5;
        }
    }
}

void iceBaddie_handleMessage(GameObject* obj, int message) {
    GroundBaddieState* state = obj->extra;

    switch ((u8)message) {
    case 0x80:
        (*gPlayerInterface)->setState(obj, state, 2);
        state->baddie.substate = 4;
        state->baddie.moveJustStartedB = 1;
        break;
    }
}

s16 iceBaddie_getControlMode(GameObject* obj) {
    return ((GroundBaddieState*)obj->extra)->baddie.controlMode;
}

int iceBaddie_getExtraSize(void) {
    return sizeof(GroundBaddieState) + sizeof(IceBaddieControl);
}

int iceBaddie_getObjectTypeId(void) {
    return 0x49;
}

void iceBaddie_free(GameObject* obj) {
    GroundBaddieState* state = obj->extra;

    CameraShake_Disable();
    ObjGroup_RemoveObject((int)obj, ICEBADDIE_OBJGROUP);
    {
        GameObject* child = (GameObject*)obj->childObjs[0];
        if (child != NULL) {
            Obj_FreeObject(child);
            obj->childObjs[0] = NULL;
        }
    }
    (*gBaddieControlInterface)->releaseState(obj, state, 0x20);
}

void iceBaddie_render(GameObject* obj, int fwdArg2, int fwdArg3, int fwdArg4, int fwdArg5, s8 visible) {
    GroundBaddieState* state = obj->extra;
    f32 zero = 0.0f;

    if (visible == 0 || obj->userData1 != 0 || state->targetState == 0) {
        return;
    }

    if (state->glowAlpha != zero) {
        objSetGlowColor(0xc8, 0, 0, state->glowAlpha);
    }
    objRenderModelAndHitVolumes(obj, fwdArg2, fwdArg3, fwdArg4, fwdArg5, 1.0f);
    iceBaddie_updateEffectAnchors(obj, state);
}

void iceBaddie_hitDetect(GameObject* obj) {
    (*gPlayerInterface)->updateVelocityState(obj, obj->extra, gIceBaddieStateHandlersA);
}

void iceBaddie_update(GameObject* obj, int unusedA, int unusedB) {
    GroundBaddieState* objectState;
    IceBaddiePlacement* placement;

    (void)unusedA;
    (void)unusedB;

    objectState = obj->extra;
    placement = (IceBaddiePlacement*)obj->anim.placementData;
    if (obj->userData1 != 0) {
        if ((objectState->baddie.substate != 3 || (objectState->configFlags & 1) != 0) &&
            (*gMapEventInterface)->shouldNotSaveTime(placement->base.ident) != 0) {
            (*gBaddieControlInterface)
                ->initGroundBaddie(obj, (u8*)placement, (u8*)objectState, 14, 8, 0x102, 0x26, 20.0f);
            objectState->targetState = 0;
            Sfx_PlayFromObject((u32)obj, SFXTRIG_dn_seal4_c_263);
            ObjAnim_SetCurrentMove((int)obj, 8, 0.0f, OBJANIM_MOVE_CONTROL_SKIP_EVENT_COUNTDOWN);
            objectState->baddie.moveDone = 0;
            obj->anim.alpha = 0xff;
            obj->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
        }
    } else if (obj->userData2 == 0) {
        obj->anim.localPosX = placement->base.posX;
        obj->anim.localPosY = placement->base.posY;
        obj->anim.localPosZ = placement->base.posZ;
        (*gObjectTriggerInterface)->runSequence(placement->sequenceId, obj, -1);
        obj->userData2 = 1;
    } else {
        if ((*gBaddieControlInterface)->isObjectValid(obj, objectState, 0) == 0) {
            objectState->targetState = 0;
        } else {
            iceBaddie_updateTargetCollision(obj, (int)objectState, objectState);
            iceBaddie_updateControlEffects(obj, objectState);
            if (objectState->targetState == 0) {
                iceBaddie_tryAcquireTarget(obj, objectState, objectState);
            } else {
                iceBaddie_updateTargetMotion(obj, objectState, objectState);
            }
            if ((objectState->configFlags & 2) != 0) {
                obj->anim.localPosY = placement->base.posY - 8.0f;
            }
        }
    }
}

void iceBaddie_init(GameObject* obj, IceBaddiePlacement* placement, int flags) {
    GroundBaddieState* objectState;
    u8 mode;

    objectState = obj->extra;
    mode = 6;
    if (flags != 0) {
        mode |= 1;
    }
    if ((placement->flags & 0x20) == 0) {
        mode |= 8;
    }
    (*gBaddieControlInterface)->initGroundBaddie(obj, (u8*)placement, (u8*)objectState, 14, 8, 0x102, mode, 20.0f);
    obj->animEventCallback = NULL;
    if (0.5f * (f32)(u32)objectState->aggroRange < 55.0f) {
        objectState->aggroRange = 0x6e;
    }
    ObjAnim_SetCurrentMove((int)obj, 8, 0.0f, 0);
    obj->anim.resetHitboxFlags |= INTERACT_FLAG_DISABLED;
    (*gPlayerInterface)->setState(obj, objectState, 0);
    objectState->baddie.substate = 0;
    objectState->baddie.physicsActive = 0;
}

void iceBaddie_release(void) {
}

void iceBaddie_initialise(void) {
    iceBaddie_installStateHandlers();
}

ObjectDescriptor12 gIceBaddieObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_12_SLOTS,
    (ObjectDescriptorCallback)iceBaddie_initialise,
    (ObjectDescriptorCallback)iceBaddie_release,
    0,
    (ObjectDescriptorCallback)iceBaddie_init,
    (ObjectDescriptorCallback)iceBaddie_update,
    (ObjectDescriptorCallback)iceBaddie_hitDetect,
    (ObjectDescriptorCallback)iceBaddie_render,
    (ObjectDescriptorCallback)iceBaddie_free,
    (ObjectDescriptorCallback)iceBaddie_getObjectTypeId,
    iceBaddie_getExtraSize,
    (ObjectDescriptorCallback)iceBaddie_getControlMode,
    (ObjectDescriptorCallback)iceBaddie_handleMessage,
};
