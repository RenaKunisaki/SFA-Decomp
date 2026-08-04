# Near-match worklist — every sub-100 function, classified

Generated 2026-08-03 at HEAD `574310f3e0` (report built at `20dbc442e5` + one type-only commit),
tree fuzzy 99.81795, **205 sub-100 functions**. This replaces the old band-width-only ranking:
the fixed `tools/structscan.py` now reloc-masks and pool-canonicalises the diff, so `ndiff` below
is REAL differing instruction words (an `@N`-vs-`lbl_` pool naming difference no longer counts).

## Regeneration

    ninja build/GSAE01/report.json
    # per sub-100 fn: structscan.fn_diff(target.o, src.o, sym)  -> ndiff, struc, lengths
    # band width: max _savegpr_N/_savefpr_N reloc in the TARGET disasm (fallback: prologue stw/stfd r14..r31)
    # (script used this pass: scratchpad scan_all.py; citations = grep of docs/ + the memory dir)

Columns: `ndiff` real differing words · `struc` mnemonic changes + length delta · `band` saved
GPR/FPR width from the target's `_savegpr_N`/`_savefpr_N` · `#nm` sub-100 functions left in the
unit (1 = matching this one flips the unit).

Reading the table: **`struc 0` = pure recolour** (rotation if band >= 5, narrow-band/scratch
otherwise — neither is declaration-steerable, see the band model in CLAUDE.md);
**`struc > 0` = a shape difference**, which is where source levers apply — but nearly every one
below already has a recorded mechanism. The taxonomy and citations are from
`docs/priced_classes.md` + the memory topic files; a section-number cite (`priced N`) means
`docs/priced_classes.md` section N.

## The verdict of this pass

**After classification, ZERO functions remain UNCLASSIFIED** — every sub-100 function maps to a
recorded mechanism, a signature-clean recolour, or an accepted/priced class. The frontier is
walled at the function level; the open value is in the three leads below, the owner decisions
(pool order / naming / `.sdata2` claims), and the TU-split evidence lanes — not in fresh
function-level probing.

## LEADS — the only live threads found

| # | lead | what to do |
|---|---|---|
| 1 | **RESOLVED 2026-08-03 — player.c flags-mask records were compiler-conditioned, not contradictory.** Both `6c20352050` (negative literals -> li/and, State19/MountBike 100) and `55fb6e45c1` ("`~PLAYER_FLAG_*` byte-identical") were measured under `mw_version="GC/1.3"` (set by `b8d190fdac`); `565f6ed47d` (2026-08-03) removed that override as a compiler false set, and under the default GC/2.0 profile the two spellings are NOT equivalent: every 32-bit spelling (`~mask`, `-mask-1`, `0xFFF...`) folds to `rlwinm`, and only an LL-widened mask reaches retail's materialize-the-mask shape (`li rN,-mask; and` / `lis;addi;and`) — at the cost of a dead high-word `li rN,0` hoisted at the first LL site, which retail does not have (retail remats `li r0,0` at the consuming store). Landed: 32-bit `-3` at the HITDETECT clears in State1B/State19/MountBike/ClimbWall/player_SeqFn (drops the dead zero; rlwinm accepted) — 99.399->99.535, 99.295->99.456, 99.405->99.546, 98.647->98.715, 98.971->99.007; unit 99.7986->99.8065. Negative-literal spelling kept because it is byte-identical to `~mask` today and becomes the exact retail li/and under GC/1.3. Kept LL where measured better (playerRender `~0x100000LL` 99.506, playerDoHitDetection `~PLAYER_FLAG_WORLDPOS_OVERRIDE` 99.617; their 32-bit forms regressed to 99.401/99.528). Walled under GC/2.0: StopRidingObject 97.978 (needs li/and without the LL zero — `-0x1001` inert, `~0x1000LL` 95.04), CheckIfClimbingOntoWall 99.791 (`-0x101` inert, `~0x100LL` 99.640). Residual family diffs (li/and-vs-rlwinm + zero-remat position) are GC/1.3-only shapes; further spelling probes are dead — the open question is the compiler-version owner call, not source. |
| 2 | **`beginLoadingMap`** (main/shader, 2324 B, ndiff 7, band 3G) | Still the recorded "highest-value unresolved single-defect function on the frontier" (near100 census): one store-forward-vs-reload of `gMapBlockOriginZ`, symbol-provenance family, falsified across 5 mw_versions + 9 flag profiles. Only a genuinely new lever applies. |
| 3 | **`Effect3_spawnObject`** (engine/28, 7796 B, ndiff 14, struc 0, sole straggler) | **TERMINAL — mw_version/flag axis exhausted 2026-08-03.** Full out-of-tree matrix on the untouched 28.c: 20 compiler versions (1.0..3.0a5.2) x 6 profiles (base/nocse/noprop/nolife/noloopinv/-inline noauto) + 16 combo cells on GC/2.0 (nostrength, nodead, nocse+noprop, noprop+each, -inline off pairs). 1.0-1.2.5n and 3.0a* are different codegen eras (ndiff 682-1187, struc>0); every 1.3-2.7 cell reproduces the identical 14/0; the sole mover is `nopropagation` (13/0 at every 1.3-2.7 version, uniform), which re-rotates the scramble (`r31,r29,r27` + rlwinm r30) without approaching the canonical `r27,r29,r30` + rlwinm r31 band. Floor of the whole space = 13. The 4-cycle is compiler-internal at every available version; no per-TU flag or mw_version change flips this unit. |

Probe results this pass (both reverted, tree byte-exact at baseline afterwards): see lead 1.

## Classified inventory

Grouped by family; within a family sorted by ndiff ascending (closest to byte-exact first),
then size descending.

### symbol-provenance (14)

MWCC GC/1.3+ elides a redundant load/store-forward whenever the address's provenance is a compile-time symbol; retail reloads. No source spelling reaches the reload.

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | recorded mechanism |
|---|---|---|---|---|---|---|---|---|
| dll_98_spawnEffect | dlls/modgfx/152/152 | 1040 | 99.769 | 1 | 1 | 6G/0F | 1 | store-forward vs reload (7 spellings walled) |
| mathSinCosf | main/sincosf | 320 | 98.750 | 1 | 2 | 2G/5F | 1 | un-elided parameter-home fmr copy; profile already maximal |
| mathTanf | dolphin/MSL_C/PPCEABI/bare/H/math_8029454c | 148 | 97.297 | 1 | 2 | 0G/4F | 1 | un-elided parameter-home fmr copy; profile already maximal |
| gameUiLoadResources | dlls/engine/0/0 | 896 | 98.661 | 4 | 6 | 7G/3F | 14 | symbol-provenance-load-cse (the 6-fn reload family) |
| renderClouds | dlls/engine/9/9 | 1612 | 99.491 | 6 | 4 | 6G/1F | 1 | global-load-CSE volatile purge (renderClouds closed) |
| beginLoadingMap | main/shader | 2324 | 99.776 | 7 | 4 | 3G/0F | 8 | symbol-provenance-load-cse (the 6-fn reload family) |
| sceneDraw | main/lightmap | 1500 | 98.880 | 19 | 13 | 4G/0F | 3 | symbol-provenance-load-cse (the 6-fn reload family) |
| loadCharacter | main/object | 2612 | 99.763 | 27 | 0 | 12G/1F | 1 | loadCharacter residual after 505db2b3ce win: copy/load swap + inverted branch |
| worldplanet_update | dlls/objects/466_WORLDplanet/WORLDplanet | 3136 | 99.152 | 34 | 18 | 13G/9F | 1 | worldplanet: 4 independent defects incl. -1 elision |
| drlasercannon_aimAtTarget | dlls/objects/609_DR_LaserCan/DR_LaserCan | 940 | 97.660 | 36 | 36 | 6G/1F | 1 | double-extsh fold; fix is a main-owned getAngle prototype question |
| voxmaps_updateActiveMap | main/voxmaps | 776 | 97.412 | 38 | 33 | 8G/0F | 3 | symbol-provenance-load-cse (the 6-fn reload family) |
| ObjSeq_start | dlls/engine/2/2 | 2904 | 99.311 | 50 | 15 | 18G/3F | 5 | symbol-provenance-load-cse (the 6-fn reload family) |
| ObjSeq_runBgCmds | dlls/engine/2/2 | 952 | 97.689 | 62 | 12 | 16G/0F | 5 | remainder-remat cursor unreachable (engine/2 GROUND pass) |
| doPendingMapLoads | main/shader | 3144 | 98.458 | 164 | 100 | 14G/1F | 8 | symbol-provenance-load-cse (the 6-fn reload family) |

### copy-survival (9)

The li/mr rematerialisation family (incl. the srawi/extsh u64-pair pocket and copy-class placement): the fix flips an unsteerable band rotation, nets negative.

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | recorded mechanism |
|---|---|---|---|---|---|---|---|---|
| Scarab_update | dlls/objects/262/262 | 3476 | 99.931 | 1 | 1 | 6G/1F | 1 | copy-survival-first-definition-rule (walled; clause-A fix nets negative) |
| curves_advanceCollision | dlls/engine/21/21 | 2472 | 99.903 | 1 | 1 | 6G/1F | 1 | copy-survival-first-definition-rule (walled; clause-A fix nets negative) |
| playerStateMountBike | dlls/objects/195_Player/player | 1452 | 99.405 | 3 | 2 | 6G/0F | 22 | independent-match-ceiling: the srawi/extsh ~(u64) pocket |
| playerState19 | dlls/objects/195_Player/player | 1396 | 99.295 | 8 | 5 | 6G/1F | 22 | independent-match-ceiling: the srawi/extsh ~(u64) pocket |
| Shield_setMode | dlls/objects/229/229 | 1788 | 99.709 | 15 | 1 | 7G/4F | 2 | web-class: 226/229/engine-22 copy-class placement / copy-survival textures |
| Shield_update | dlls/objects/229/229 | 808 | 99.431 | 21 | 0 | 9G/1F | 2 | web-class: 226/229/engine-22 copy-class placement / copy-survival textures |
| staff_setupSwipe | dlls/objects/226/226 | 1780 | 99.708 | 26 | 0 | 10G/7F | 2 | web-class: 226/229/engine-22 copy-class placement / copy-survival textures |
| textRenderStr | main/textrender | 4104 | 99.693 | 50 | 1 | 12G/10F | 1 | textRenderStr walls (copy-survival cited; other-lane file) |
| dll_0B_spawnEffect | dlls/engine/11/11 | 2432 | 98.998 | 72 | 3 | 11G/0F | 3 | flip-frontier: li;mr produced by the off=i idiom; residual scratch perm |

### const-zero-remat (7)

Retail materialises a constant zero at its store; we hoist it into a web (or vice versa). All spellings inert — naming is coalesced away.

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | recorded mechanism |
|---|---|---|---|---|---|---|---|---|
| intersectModLineBuild | main/track_dolphin | 1352 | 99.822 | 1 | 1 | 8G/1F | 7 | track-objhits: textbook const-zero-remat (T mr vs C li) |
| wispBaddieProcessAnimEvent | dlls/objects/202/sharpclaw | 1000 | 99.156 | 3 | 1 | 5G/0F | 1 | dll202 doc: retail remats li r0,0 at store, we hoist |
| playerDoHitDetection | dlls/objects/195_Player/player | 2244 | 99.617 | 6 | 4 | 3G/2F | 22 | const-zero-remat-residual / array-clear wall |
| playerRender | dlls/objects/195_Player/player | 1904 | 99.506 | 7 | 3 | 7G/3F | 22 | same hoist-vs-remat shape verified this session (see LEAD 1) |
| playerState1B | dlls/objects/195_Player/player | 1636 | 99.399 | 8 | 5 | 4G/1F | 22 | same hoist-vs-remat shape verified this session (see LEAD 1) |
| updateEnvironment | main/lightmap | 608 | 99.276 | 11 | 1 | 2G/0F | 3 | const-zero-remat-residual / array-clear wall |
| textureInitGXTexObj | main/texture | 356 | 98.427 | 27 | 0 | 4G/0F | 3 | const-zero-remat-residual / array-clear wall |

### store-truncate (3)

Retail-deleted redundant extsh/extsb before a narrow store; unreachable without per-function peephole or a prototype change that breaks callers.

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | recorded mechanism |
|---|---|---|---|---|---|---|---|---|
| babyCloudRunner_turnTowardTarget | dlls/objects/332/332 | 352 | 98.864 | 1 | 2 | 4G/0F | 1 | priced 4b / redundant-narrowing store-truncate wall |
| playerStateClimbOntoLadder | dlls/objects/195_Player/player | 1396 | 99.699 | 2 | 2 | 7G/1F | 22 | playerSetMoveBlendFromPlane return-type pincer (web-class 08-03); verified this session: both callers' sole real diff is one surplus extsh before sth |
| playerStateClimbOntoWall | dlls/objects/195_Player/player | 736 | 99.429 | 2 | 2 | 4G/0F | 22 | playerSetMoveBlendFromPlane return-type pincer (web-class 08-03); verified this session: both callers' sole real diff is one surplus extsh before sth |

### unrolled-IV/unroll-CSE (4)

Unrolled-loop base-chunking / stack-slot + unroll interactions; retail folds strides into displacements where we re-base.

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | recorded mechanism |
|---|---|---|---|---|---|---|---|---|
| mmFreeTick | main/mm | 788 | 98.721 | 13 | 13 | 3G/0F | 2 | unrolled-iv-fold-symbol-base-law |
| SaveGame_gplaySetObjGroupStatus | dlls/engine/23/23 | 1056 | 97.981 | 30 | 25 | 9G/0F | 1 | priced 3 / W2 unroll base-chunking |
| Objfsa_UpdateWalkGroupPatches | dlls/engine/20_Hcurves/Hcurves | 4776 | 99.491 | 72 | 48 | 11G/7F | 1 | priced 3 / W2 unroll base-chunking |
| expgfx_updateActivePools | dlls/engine/10_expgfx/expgfx | 9252 | 99.219 | 122 | 116 | 18G/10F | 4 | surplus-queue: stack-slot layout + unroll block, needs dedicated session |

### ternary-constant-register (1)

Ternary/constant coalescing: both polarities lose.

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | recorded mechanism |
|---|---|---|---|---|---|---|---|---|
| DIMCannon_updateAim | dlls/objects/454_DIMCannon/DIMCannon | 780 | 99.769 | 8 | 1 | 6G/2F | 1 | flip-frontier: 23 clamp spellings flat |

### goto-locked (2)

Retail shape reachable only through goto (or its comma-condition disguise) — banned; unit stays NonMatching by design.

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | recorded mechanism |
|---|---|---|---|---|---|---|---|---|
| mmpMoonRock_update | dlls/objects/386_MMP_moonroc/MMP_moonroc | 1736 | 99.919 | 6 | 0 | 5G/1F | 1 | structscan doc: only working spelling is a goto-in-disguise; accepted NonMatching |
| playerStateClimbWall | dlls/objects/195_Player/player | 2708 | 98.647 | 144 | 21 | 9G/4F | 22 | goto-recovery-candidate-pool (144 real words, 21 mnemonic) |

### per-function-opt-level (9)

Function matches under a different opt profile (peephole/nocse/nopropagation/dead/level) that craters its TU siblings; per-fn pragma banned. Accepted.

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | recorded mechanism |
|---|---|---|---|---|---|---|---|---|
| atan2fHighPrecision | main/acosf | 480 | 99.125 | 2 | 3 | 1G/5F | 3 | priced 1 dot-compare (peephole) family |
| atan2f | main/acosf | 272 | 98.456 | 2 | 3 | 1G/5F | 3 | priced 1 dot-compare (peephole) family |
| atan2f_fast | main/acosf | 240 | 98.250 | 2 | 3 | 1G/5F | 3 | priced 1 dot-compare (peephole) family |
| playerCacheMoveRootHeights | dlls/objects/195_Player/player | 352 | 97.045 | 3 | 5 | 5G/0F | 22 | player-cluster: 100.0 under nocse/nopropagation, TU-split evidence needed |
| removeButtonObject | main/gameloop_buttonobj | 220 | 98.091 | 3 | 3 | 0G/0F | 1 | priced 1 dot-compare (peephole) family |
| moveTricky | dlls/objects/196_Tricky/tricky | 2404 | 99.933 | 6 | 0 | 7G/2F | 6 | surplus-queue: -opt dead fixes it, craters siblings |
| subtitleUpdateAndDraw | main/subtitle | 536 | 97.799 | 12 | 8 | 3G/0F | 1 | biased-base follow-up: source proven original, level compromise |
| pathcam_buildWindowSamples | dlls/engine/71/71 | 1220 | 98.820 | 52 | 0 | 18G/0F | 1 | residual pass 08-03: byte-identical under nocse |
| playerUpdate | dlls/objects/195_Player/player | 2372 | 98.432 | 147 | 2 | 3G/1F | 22 | player-cluster: 100.0 under nocse/nopropagation, TU-split evidence needed |

### owner-gated (13)

.sdata2 mint order / pool naming / named-slot families blocked on CLAUDE.md's pool-order clause or a naming owner call.

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | recorded mechanism |
|---|---|---|---|---|---|---|---|---|
| fsin16HighPrecision | main/trig | 416 | 99.981 | 2 | 0 | 1G/3F | 8 | priced 6b/A77 + trig-cluster-stackslot: pool order + stack-slot residual |
| fcos16HighPrecision | main/trig | 416 | 99.981 | 2 | 0 | 1G/3F | 8 | priced 6b/A77 + trig-cluster-stackslot: pool order + stack-slot residual |
| fsin16Precise | main/trig | 328 | 99.976 | 2 | 0 | 1G/2F | 8 | priced 6b/A77 + trig-cluster-stackslot: pool order + stack-slot residual |
| fcos16Precise | main/trig | 328 | 99.976 | 2 | 0 | 1G/2F | 8 | priced 6b/A77 + trig-cluster-stackslot: pool order + stack-slot residual |
| fsin16 | main/trig | 296 | 99.973 | 2 | 0 | 1G/2F | 8 | priced 6b/A77 + trig-cluster-stackslot: pool order + stack-slot residual |
| fcos16 | main/trig | 296 | 99.973 | 2 | 0 | 1G/2F | 8 | priced 6b/A77 + trig-cluster-stackslot: pool order + stack-slot residual |
| fsin16Approx | main/trig | 264 | 99.970 | 2 | 0 | 1G/2F | 8 | priced 6b/A77 + trig-cluster-stackslot: pool order + stack-slot residual |
| fcos16Approx | main/trig | 264 | 99.970 | 2 | 0 | 1G/2F | 8 | priced 6b/A77 + trig-cluster-stackslot: pool order + stack-slot residual |
| CameraModeTalk_update | dlls/engine/69/69 | 1076 | 99.870 | 6 | 0 | 2G/3F | 1 | priced 8b intra-function mint order (engine/69, 64 B) |
| drakorhoverpad_updateMain | dlls/objects/625/625 | 1532 | 99.843 | 12 | 0 | 4G/2F | 1 | web-class: retail slot is the TU's anonymous pool literal we NAMED |
| staffUpdateSegmentTransforms | main/objprint | 664 | 97.018 | 17 | 7 | 9G/0F | 2 | pool-allocation-order class |
| sky2_run | dlls/engine/6/6 | 2824 | 99.653 | 37 | 19 | 6G/9F | 1 | priced regression-class: fcmpo definition-order pinned (invcmp_scan) |
| renderSunAndMoon | dlls/engine/5/5 | 1948 | 98.840 | 60 | 7 | 6G/3F | 1 | priced 18/9: residual after renderSunAndMoon 98.84 close; pool noise |

### rotation (68)

Width >=5 saved band, identical mnemonic stream: the rotation-offset model — every ordering knob provably flat. Do not sweep.

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | recorded mechanism |
|---|---|---|---|---|---|---|---|---|
| trackSweepCircleAgainstPoint | main/track_dolphin | 656 | 99.939 | 2 | 0 | 3G/7F | 7 | track-objhits-coloring-walls (T==C, derived to ground) |
| debugPrintfxy | main/dll_80136a40 | 424 | 99.387 | 2 | 0 | 7G/0F | 4 | signature: struc 0, band >=5, no recorded lever site |
| trickyGuard | dlls/objects/196_Tricky/tricky | 2276 | 99.947 | 6 | 0 | 7G/0F | 6 | near100-band-census-2026-08-01 (16 dead ends) |
| RomCurve_findShortestPathLink | dlls/engine/20_Hcurves/Hcurves_romcurve | 1572 | 99.911 | 6 | 0 | 15G/1F | 1 | refreshed-flip-frontier 08-02 (width noted per fn) |
| ObjHitbox_SetStateIndex | main/objhits | 140 | 98.857 | 6 | 0 | 0G/0F | 9 | objhits GROUND pass 08-03: all 9 T==C recolours, no lever site |
| waterfx_drawSplashBurst | dlls/engine/19/19 | 664 | 99.789 | 7 | 0 | 5G/12F | 2 | signature: struc 0, band >=5, no recorded lever site |
| pauseMenuDraw | dlls/engine/0/0 | 4564 | 99.956 | 8 | 0 | 5G/0F | 14 | signature: struc 0, band >=5, no recorded lever site |
| expgfx_addremove | dlls/engine/10_expgfx/expgfx | 2576 | 99.938 | 8 | 0 | 10G/0F | 4 | signature: struc 0, band >=5, no recorded lever site |
| ObjHits_CollectSkeletonHitsXZ | main/objhits | 1124 | 99.786 | 8 | 0 | 15G/7F | 9 | objhits GROUND pass 08-03: all 9 T==C recolours, no lever site |
| InvHit_update | dlls/objects/241_InvHit/InvHit | 1024 | 99.844 | 8 | 0 | 5G/3F | 1 | refreshed-flip-frontier 08-02 (width noted per fn) |
| ObjHits_CollectSkeletonHits3D | main/objhits | 988 | 99.757 | 8 | 0 | 15G/5F | 9 | objhits GROUND pass 08-03: all 9 T==C recolours, no lever site |
| mapProcessRomList | main/shader | 560 | 99.643 | 8 | 0 | 5G/2F | 8 | signature: struc 0, band >=5, no recorded lever site |
| trickyFindReachableRouteIndex | dlls/objects/196_Tricky/tricky | 468 | 99.573 | 8 | 0 | 8G/0F | 6 | surplus-queue tricky walls (5 exhaustive sweeps flat) |
| modelInitBoneMtxs | main/model | 236 | 99.288 | 8 | 0 | 6G/0F | 8 | signature: struc 0, band >=5, no recorded lever site |
| modelDoRenderInstrs | main/objprint_dolphin | 3160 | 99.943 | 9 | 0 | 15G/4F | 6 | biased-base-add-canon 08-03 (residual width-15 rotation + pool naming) |
| tricky_updateBallRoll | dlls/objects/196_Tricky/tricky | 1516 | 99.881 | 9 | 0 | 7G/1F | 6 | surplus-queue tricky walls (5 exhaustive sweeps flat) |
| objInterpretSeq | dlls/objects/294/294 | 3920 | 99.939 | 10 | 0 | 10G/0F | 1 | near100-band-census-2026-08-01 (16 dead ends) |
| renderShadows | main/newshadows | 2596 | 99.730 | 10 | 2 | 18G/11F | 6 | near100-band-census-2026-08-01 (16 dead ends) |
| objJointTracksAimAtTarget | main/objprint | 1332 | 99.820 | 10 | 0 | 8G/4F | 2 | track-objhits-coloring-walls (T==C, derived to ground) |
| defragMemory | main/pi_dolphin | 1028 | 99.767 | 11 | 0 | 11G/0F | 2 | signature: struc 0, band >=5, no recorded lever site |
| CameraModeWorldMap_update | dlls/engine/78/78 | 3212 | 99.925 | 12 | 0 | 7G/4F | 1 | near100-band-census-2026-08-01 (16 dead ends) |
| SnowBike_UpdateTrails | dlls/objects/597/597 | 1600 | 99.825 | 12 | 0 | 14G/4F | 3 | signature: struc 0, band >=5, no recorded lever site |
| padUpdate | main/pad | 1380 | 99.768 | 12 | 0 | 18G/0F | 1 | sole-straggler-frontier (clean saved-band swaps width 6-15) |
| modelLoadAnimations | main/model | 944 | 99.661 | 13 | 0 | 6G/0F | 8 | signature: struc 0, band >=5, no recorded lever site |
| Effect3_spawnObject | dlls/engine/28/28 | 7796 | 99.962 | 14 | 0 | 6G/1F | 1 | TERMINAL 2026-08-03: mw_version x flag matrix exhausted (136 cells), floor 13 under noprop, 4-cycle compiler-internal at every version — see lead 3 |
| bossdrakor_update | dlls/objects/589_BossDrakor/BossDrakor | 2192 | 99.854 | 14 | 0 | 7G/2F | 2 | near100-band-census-2026-08-01 (16 dead ends) |
| mapInstantiateObjects | main/shader | 556 | 99.460 | 14 | 0 | 7G/0F | 8 | signature: struc 0, band >=5, no recorded lever site |
| mapSetup | main/shader | 408 | 99.167 | 16 | 0 | 5G/1F | 8 | independent-match-ceiling (permsweep walls) |
| dimlavasmash_setBlockSurfaceFlags | dlls/objects/455_DIMLavaSmas/DIMLavaSmas | 276 | 98.841 | 16 | 0 | 6G/0F | 1 | refreshed-flip-frontier 08-02 (width noted per fn) |
| debugPrintDrawRecord | main/dll_80136a40 | 1824 | 99.792 | 17 | 0 | 10G/3F | 4 | near100-band-census-2026-08-01 (16 dead ends) |
| trackGetLineIntersect | main/track_dolphin | 1656 | 99.758 | 17 | 0 | 18G/4F | 7 | track-objhits-coloring-walls (T==C, derived to ground) |
| playerBuildLedgeClimbProbe | dlls/objects/195_Player/player | 1296 | 99.691 | 19 | 0 | 15G/3F | 22 | signature: struc 0, band >=5, no recorded lever site |
| ObjSeq_ExecuteActionCommand | dlls/engine/2/2 | 2012 | 99.742 | 22 | 0 | 7G/0F | 5 | near100-band-census-2026-08-01 (16 dead ends) |
| ObjHits_CheckTrackContact | main/objhits | 1068 | 99.551 | 23 | 0 | 9G/0F | 9 | objhits GROUND pass 08-03: all 9 T==C recolours, no lever site |
| objFuzzRenderCb | main/objprint_dolphin | 2780 | 99.827 | 24 | 0 | 5G/1F | 6 | signature: struc 0, band >=5, no recorded lever site |
| dll_0B_renderEffects | dlls/engine/11/11 | 2512 | 99.745 | 24 | 0 | 12G/2F | 3 | near100-band-census-2026-08-01 (16 dead ends) |
| CameraModeNormal_updateWallAvoidance | dlls/engine/66/66 | 1280 | 99.594 | 24 | 0 | 6G/7F | 1 | refreshed-flip-frontier 08-02 (width noted per fn) |
| ObjHits_DetectObjectPair | main/objhits | 1232 | 99.529 | 24 | 0 | 5G/8F | 9 | objhits GROUND pass 08-03: all 9 T==C recolours, no lever site |
| mapFillCellEntry | main/shader | 752 | 99.324 | 26 | 0 | 8G/0F | 8 | signature: struc 0, band >=5, no recorded lever site |
| dbstealerworm_stateHandlerA0B | dlls/objects/578_DBstealerwo/DBstealerwo | 1300 | 99.585 | 27 | 0 | 9G/2F | 1 | web-class-pun-effect (width 17 + copy-class flip / width 9) |
| modelCalcVtxGroupMtxs | main/objprint_dolphin | 636 | 99.151 | 27 | 0 | 9G/5F | 6 | signature: struc 0, band >=5, no recorded lever site |
| ObjHits_Update | main/objhits | 1736 | 99.604 | 28 | 0 | 13G/1F | 9 | objhits GROUND pass 08-03: all 9 T==C recolours, no lever site |
| voxmaps_updateRoutePath | main/voxmaps | 1192 | 99.430 | 28 | 0 | 6G/0F | 3 | objseq-voxmaps-walls (rotation offset 2/3rd unreachable) |
| Link_render | dlls/engine/60/60 | 1128 | 99.468 | 28 | 0 | 9G/1F | 1 | sole-straggler-frontier (clean saved-band swaps width 6-15) |
| ObjModel_BlendVertexStream | main/model | 628 | 99.013 | 28 | 0 | 14G/0F | 8 | independent-match-ceiling (permsweep walls) |
| titleScreenDrawMenuFrame | dlls/objects/704/704 | 2772 | 99.488 | 29 | 1 | 10G/2F | 1 | near100-band-census-2026-08-01 (16 dead ends) |
| ObjModel_BlendNormalStream | main/model | 880 | 99.182 | 29 | 0 | 16G/0F | 8 | signature: struc 0, band >=5, no recorded lever site |
| trackIntersect | main/track_dolphin | 2280 | 99.553 | 32 | 1 | 16G/7F | 7 | near100-band-census-2026-08-01 (16 dead ends) |
| trickyUpdateMovementState | dlls/objects/196_Tricky/tricky | 8764 | 99.922 | 34 | 0 | 11G/2F | 6 | near100-band-census-2026-08-01 (16 dead ends) |
| tricky_SeqFn | dlls/objects/196_Tricky/tricky | 1168 | 99.384 | 35 | 0 | 8G/0F | 6 | surplus-queue tricky walls (5 exhaustive sweeps flat) |
| addShaderLayerStages | main/objprint_dolphin | 1128 | 99.184 | 38 | 0 | 13G/0F | 6 | signature: struc 0, band >=5, no recorded lever site |
| StaffCollision_spawn | dlls/modgfx/90/90 | 1408 | 99.219 | 39 | 1 | 17G/7F | 1 | web-class-pun-effect (width 17 + copy-class flip / width 9) |
| dll_0B_updateActiveEffects | dlls/engine/11/11 | 3420 | 99.749 | 40 | 0 | 13G/0F | 3 | signature: struc 0, band >=5, no recorded lever site |
| ObjHits_CheckSkeletonPair | main/objhits | 1116 | 99.247 | 41 | 0 | 5G/0F | 9 | objhits GROUND pass 08-03: all 9 T==C recolours, no lever site |
| objSetupRenderOpGxState | main/objprint_dolphin | 1976 | 99.524 | 43 | 0 | 13G/0F | 6 | signature: struc 0, band >=5, no recorded lever site |
| trackGetIntersect2 | main/track_dolphin | 4460 | 99.762 | 47 | 0 | 18G/11F | 7 | near100-band-census-2026-08-01 (16 dead ends) |
| loadGameTextSequence | main/textrender_run | 588 | 98.061 | 47 | 0 | 8G/0F | 4 | independent-match-ceiling (permsweep walls) |
| SHthorntail_update | dlls/objects/429_SH_thorntai/SHthorntail | 1928 | 99.274 | 69 | 0 | 6G/1F | 1 | sole-straggler-frontier (clean saved-band swaps width 6-15) |
| SB_Galleon_updateFlight | dlls/objects/488_SB_Galleon/SB_Galleon | 5732 | 99.749 | 71 | 0 | 5G/10F | 1 | near100-band-census-2026-08-01 (16 dead ends) |
| ObjHits_CheckObjectHitVolumes | main/objhits | 1392 | 98.951 | 73 | 0 | 11G/1F | 9 | objhits GROUND pass 08-03: all 9 T==C recolours, no lever site |
| gameTextWrapLines | main/gametext_tail | 1836 | 98.932 | 84 | 0 | 12G/3F | 1 | sole-straggler-frontier (clean saved-band swaps width 6-15) |
| voxmaps_visitRouteNeighbor | main/voxmaps | 2296 | 98.990 | 98 | 0 | 18G/0F | 3 | objseq-voxmaps-walls (rotation offset 2/3rd unreachable) |
| ObjHits_CheckHitVolumes | main/objhits | 3592 | 99.382 | 101 | 0 | 18G/18F | 9 | objhits GROUND pass 08-03: all 9 T==C recolours, no lever site |
| mapLoadDataFile | main/pi_dolphin | 8444 | 99.711 | 117 | 0 | 10G/0F | 2 | near100-band-census-2026-08-01 (16 dead ends) |
| playerBuildWallTransitionProbe | dlls/objects/195_Player/player | 1816 | 98.476 | 135 | 1 | 18G/3F | 22 | web-class-pun-effect 08-03 (one rotation off, width 18) |
| trackBuildBlockTriangles | main/track_dolphin | 3060 | 98.476 | 148 | 2 | 18G/2F | 7 | signature: struc 0, band >=5, no recorded lever site |
| ObjSeq_update | dlls/engine/2/2 | 3912 | 98.569 | 274 | 0 | 9G/4F | 5 | objseq-voxmaps-walls (rotation offset 2/3rd unreachable) |
| player_SeqFn | dlls/objects/195_Player/player | 7416 | 98.971 | 334 | 104 | 11G/3F | 22 | player-cluster-walls (width 11) |

### scratch-perm/flag-signature (33) — SUBCLASSED 2026-08-03

Former catch-all, split by a sequential out-of-tree probe of every member: unit's exact cflags,
toggling each of nocse/nopropagation/nolifetimes/noloopinvariants against the configured -opt,
-inline auto<->noauto, and mw_version neighbours (GC/1.3, 2.0, 2.5, 2.7) at base profile; per-cell
fn_diff vs the retail carve. Refined classes: **per-fn-opt-level (accepted)** = some cell byte-exact
while siblings break there (STOP probing; same trade as engine/71) — 2 members;
**multi-role-local candidate** = a named local fed from 2+ sources sits exactly in the permuted
region (mergeTableFiles precedent; flagged for a fix lane, NOT yet fixed) — 1 member
(objRenderModel; the other 6 were probed with measured split/merge rewrites, every one flat or
regressing, and are reclassified in place as split-lever-SPENT scratch rotations);
**true scratch rotation** = no cell reaches byte-exact and no multi-role local — 30 members.

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | refined class + evidence cell |
|---|---|---|---|---|---|---|---|---|
| objRenderModel | main/objprint_dolphin | 612 | 99.804->100 | 6->0 | 0 | 3G/0F | 6 | **MATCHED from source**: keep `alpha` for the two compares but spell the else-arm store `shadowAlpha += shadowAlphaStep` (CSE folds it into the one add and the re-spelled occurrence moves base/lbz/lha temps to retail r3/r0/r4); the multi-role split itself was NOT the lever — every alpha split/merge probed 6-7 diffs |
| SaveSelectScreen_render | dlls/engine/53/53 | 976 | 99.816 | 7 | 0 | 8G/0F | 1 | true scratch rotation: addi-base/li-0 cursor pair swapped (r26/r28); all cells 7/0 except +noprop worse |
| mapBlockRender_setShader | main/tex_dolphin | 968 | 99.814 | 8 | 0 | 1G/0F | 3 | true scratch rotation: fogColor temp vs byteBase r6/r7 scratch swap; flat 8/0, +noprop 15/6 worse |
| mapRomListFindItem | main/lightmap | 220 | 99.000 | 8 | 0 | 1G/0F | 3 | true scratch rotation: page/pageCursor r8/r9 swap; ALL 9 cells identical 8/0 (fully flat axis) |
| shadowVolumeBeginFrame | main/shadow_dolphin | 140 | 98.857 | 8 | 0 | 0G/0F | 2 | per-fn-opt-level (accepted): BYTE-EXACT under +nopropagation; craters buildShadowVolumeBox/initTextures/objDrawGroundShadow (17->15 unit controls); 08-03 flag-cell diff lane: the cell is a pure r4<->r5 home swap (zero-const web vs selectedBuffer lwzx web) at identical stream; 10 prop-on spellings (decl swap, literal/named zero, CSE-refold extra occurrence, block scopes both locals, int-typed zero, CF08-reload respell) all flat 8/0, double-buffers-read respell 10/0 worse — rotation confirmed at cell level, source lane closed |
| camcontrol_applyState | dlls/engine/1_camcontrol/camcontrol | 1340 | 99.851 | 9 | 0 | 2G/1F | 1 | per-fn-opt-level (accepted): BYTE-EXACT under +nopropagation; craters 7 matching siblings (42->36 unit controls), unit flip net negative; 08-03 flag-cell diff lane: the byte-exact cell reproduces ONLY against the pre-792fbf6e2d `clamped` spelling (current literal spelling's noprop cell == base, 9/0); the 9 diffs are a pure f2<->f3 swap at identical stream — outer blendProgress condition-load web (reused as the fnmsubs addend) vs the clamped phi-web (0.0f pool load shared with the outer compare, 1.0f load, fmr prog); 8 prop-on spellings (prog hoisted into the condition with literal and with clamped restored, clamped decl orders, clamped fn-scope hoist, clamped-init-before-if comparing the condition against clamped, init-position moves) all flat 9/0 — rotation confirmed at cell level, source lane closed |
| bossdrakor_updateHeadTracking | dlls/objects/589_BossDrakor/BossDrakor | 524 | 99.427 | 9 | 0 | 3G/0F | 2 | split lever SPENT -> true scratch rotation: pre-branch min hoist drops the bge/b pair (145 instrs), named frameLimit flips the ternary coalesce direction (145), decl order flat; residual is a pure delta/limit/neckStep r0-r4-r5 3-cycle |
| staff_update | dlls/objects/226/226 | 756 | 99.709 | 10 | 0 | 2G/0F | 2 | true scratch rotation: j/startIndex-temp r4/r5 anti-swap; flat 10/0; inline-toggle and GC/1.3 destroy (inline regime) |
| lightmapQueueShadowRow | main/lightmap_draw | 312 | 98.910 | 11 | 0 | 3G/0F | 1 | true scratch rotation: whole-function FP scratch rotation; flat 11/0 (confirms the 119-perm sweep) |
| powf | dolphin/MSL_C/PPCEABI/bare/H/exponentialsf | 1916 | 99.781 | 12 | 0 | 0G/0F | 1 | true scratch rotation: commutative fadds operand canon + one temp home; all 1.2.5n cells flat 12/0, GC/1.3+ wrong era (277+); MSL source-spelling lane only |
| pathSearchAddNeighbor | main/pi_pathsearch | 1092 | 99.780 | 12 | 0 | 8G/0F | 1 | split lever SPENT -> true scratch rotation: `heapIndex` is already ONE r31 web matching retail; a per-role siftIndex copy survives as a real mr (+1 instr); residual is parent/nodeIndex r5r6 vs addr-temps r8r9 swap (retail's parent reuses entry's dead r5) |
| gameTextRun | main/textrender_run | 1504 | 99.676 | 18 | 0 | 6G/2F | 4 | merge lever SPENT -> true scratch rotation: the counters are already ONE shared local `i` (MWCC splits webs per loop regardless; three constant inits can never form one web); decl-position move flat; width-6 saved band = rotation cliff, ordering knobs provably inert |
| Vortex_init | dlls/objects/691/691 | 684 | 99.415 | 18 | 0 | 6G/0F | 1 | split lever SPENT -> true scratch rotation: per-loop block-scoped split of `i` is byte-flat (i/r26+r27 already match; the permuted homes are the LICM'd table pointers per branch plus the o/base r28-r31 swap rooted in copy-emission order) |
| SnowBike_UpdateSwingBlend | dlls/objects/597/597 | 620 | 99.161 | 21 | 0 | 3G/0F | 3 | true scratch rotation: copy-class pair (param `o` / `hitResult` return) direction flip, narrow 3G; flat 21/0, GC/1.3 22/1 |
| waterfx_render | dlls/engine/19/19 | 860 | 99.428 | 24 | 0 | 7G/1F | 2 | split lever SPENT -> true scratch rotation: per-loop offset split scrambles (18->38 regions); wake-loop comma reorder (j-first init, pool/desc/vtx/j latch) reproduces retail's copy direction+update order yet scores WORSE (fn 99.428->99.279) — colors still 3-cycled |
| seqStop | musyx/runtime/synth_queue | 464 | 98.491 | 28 | 0 | 6G/0F | 1 | true scratch rotation: retail folds runtimeView->voice into one web, ours splits + 4-value loop cluster rotation; 1.2.5n confirmed right era, all alternates worse |
| ObjAnim_SampleRootCurvePhase | main/objanim | 1140 | 99.386 | 30 | 0 | 0G/0F | 1 | split lever SPENT -> true scratch rotation: per-role blendMoveData split regresses (28->31 regions, +1 instr); residual is a state/blend-moveData r5-r8 color swap (ours coalesces state into dead bank's r5, retail mints r8) plus its downstream cascade |
| expgfx_updateSourceFrameFlags | dlls/engine/10_expgfx/expgfx | 248 | 95.403 | 32 | 0 | 0G/0F | 4 | true scratch rotation, source-lane CLOSED: retail folds `flagWalk`'s init into `addi r11,r6,@lo` with the web ranked AFTER the five body scratch webs (body r6-r10, cursor r11); ours fuses web=lis-temp at r6 and shifts the whole window; +noprop separates the web (mr r10,r6, 63!=62 insns = the 10/8 cell); measured: cursor home is decl-rank-keyed among named pointer webs r4/r5/r6 only (flagWalk-before-poolIndex moves it r6->r5, 35 diffs) — indexed/SR spellings mint the cursor early and evict poolSourceIds via a surviving mr (63 insns); init-order swap, decl-initializer, for-clause init, +poolIndex, register, frameBit hoist, signedPoolIndex block push all flat/worse; no source ordering ranks a function-scope web after anonymous body temps (R3 rank-last stops at the named list) |
| gameTextBuildSystemFontAtlas | main/textrender_run | 1100 | 99.029 | 35 | 0 | 11G/0F | 4 | true scratch rotation: scratch temps + two instruction slides in the 32B copy loop; flat 35/0, +noprop 97/31 worse; other lane holds gametext |
| renderGlows | main/tex_dolphin | 1640 | 99.561 | 36 | 0 | 6G/1F | 3 | true scratch rotation: single fneg compiler temp f2-vs-f0 (no named local behind it) + pool naming noise; flat 36/0, every alternate worse |
| mtxRotateByVec3s | main/vecmath | 800 | 98.785 | 38 | 0 | 2G/5F | 1 | true scratch rotation: priced #82 FP-perm cap confirmed; flat 38/0, +noprop 48 / +nolifetimes 60 worse |
| screenTransition_drawWhiteWipe | dlls/engine/22/22 | 1288 | 99.301 | 40 | 0 | 10G/0F | 1 | true scratch rotation: saved band r22-r30 wholesale perm (width 10 rotation regime); flat 40/0 incl. level=3 kept, alternates worse |
| createNewShadowDistortionTexture | main/newshadows | 464 | 96.207 | 42 | 0 | 0G/0F | 6 | true scratch rotation: LICM pool-load discovery order + FP temp rotation (0G/0F); flat 42/0, alternates worse |
| SnowBike_UpdateEngineFx | dlls/objects/597/597 | 1080 | 99.167 | 45 | 0 | 4G/1F | 3 | true scratch rotation, source-lane CLOSED: the 45 diffs are one saved-band rotation by 2 of the 4 copy-class homes (ours obj/state/intensity/flags = r30/r31/r28/r29, retail r28/r29/r30/r31); +nolifetimes reaches retail's rotation but mints a `vol` web r5-vs-r0 (the 7 residual) so retail is lifetimes-ON at rotation 0 — a pressure state, not a flag; probed flat at identical stream: vol split, channelVol/channelVol4 delete, typed `st` local (both decl positions), decl reversal+pulse-first, windVol/fv block push, windVol if/else respell; stream-breaking (rejected): ternary->if/else, fv roundtrip removal, obj alias; param-list reorder blocked by prototype+callers; per R4 the copy-class rotation is not orderable — mw_version/TU lane only |
| CameraModeViewfinder_update | dlls/engine/68/68 | 1452 | 99.339 | 47 | 0 | 4G/0F | 1 | true scratch rotation: single param-copy home r31-vs-r30 repeated 47x; flat 47/0, inline-toggle 96 worse |
| trackResolveSurfacePenetration | main/track_dolphin | 1076 | 99.279 | 49 | 0 | 3G/1F | 7 | true scratch rotation: then/else arm ORDER + operand roles swapped in the dot-product pair (struc-0 block swap); flat 49/0, +nocse 77 worse |
| seqInit | musyx/runtime/synth_seq_dispatch | 560 | 97.643 | 53 | 0 | 0G/0F | 1 | true scratch rotation: runtime/voice cursor pair r5/r7 swap; 1.2.5n cells flat 53/0, GC/1.3+ wrong era (121/108) |
| collectShadowTrackTriangles | main/tex_dolphin | 1152 | 98.507 | 78 | 0 | 10G/2F | 3 | true scratch rotation: saved band shifted by one across r22-r27+r31 (width 10 rotation regime); flat 78/0, alternates worse |
| newShadowsInitProceduralTextures | main/newshadows | 1480 | 98.649 | 82 | 0 | 13G/8F | 6 | true scratch rotation: width-13 band rotation (r20/r21/r25/r26/r27) + one li slide; flat 82/0, alternates worse |
| gameTextInitBoxTextures | main/textrender_boxtex | 936 | 96.662 | 137 | 1 | 3G/0F | 1 | true scratch rotation: r0-temp routing in the unrolled lhzx/sth block (naming proven inert); heavy configured profile confirmed — removing ANY of its 4 tokens is worse; flat 137/1 across inline/mw |
| blendTextures | main/newshadows | 924 | 94.481 | 153 | 0 | 8G/0F | 6 | true scratch rotation: whole-band recolour; flat 153/0, every alternate worse |
| boxBlurTexture | main/newshadows | 5424 | 99.167 | 161 | 1 | 8G/0F | 6 | true scratch rotation: flat 161/1; alternates catastrophic (+nolifetimes 1109, +noloopinv 1011) |
| allocLotsOfTextures | main/newshadows | 5948 | 98.067 | 370 | 0 | 8G/16F | 6 | true scratch rotation: 8G/16F whole-band recolour; flat 370/0, +noprop 388/4, alternates worse |

### misc-compiler-wall (36)

Recorded compiler-internal walls that fit no named family: priced_classes 15 order bucket, large-const-HI (priced 4), r0 base-mat LO-fold, block-placement trampolines, preheader linearisation, engine/0 HUD cluster, etc.

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | recorded mechanism |
|---|---|---|---|---|---|---|---|---|
| errorThreadFunc | main/dll_80136a40 | 2776 | 99.954 | 2 | 0 | 8G/0F | 4 | sole-straggler: preheader statement/const-group linearisation, 23 spellings |
| playerStateAimStaff | dlls/objects/195_Player/player | 1824 | 99.967 | 3 | 0 | 6G/3F | 22 | priced 4 large-const-HI never CSE'd across a call |
| playerState30 | dlls/objects/195_Player/player | 1500 | 99.960 | 3 | 0 | 6G/1F | 22 | priced 4 large-const-HI never CSE'd across a call |
| playerStateShootFireball | dlls/objects/195_Player/player | 1132 | 99.947 | 3 | 0 | 6G/2F | 22 | priced 4 large-const-HI never CSE'd across a call |
| playerStateTryCastSpell | dlls/objects/195_Player/player | 964 | 99.938 | 3 | 0 | 4G/1F | 22 | priced 4 large-const-HI never CSE'd across a call |
| loadTextureFiles | main/texture | 436 | 97.248 | 3 | 6 | 1G/0F | 3 | block-placement: dead unreachable trampolines / dead inline preheaders |
| newclouds_run | dlls/engine/7/7 | 2376 | 99.946 | 4 | 0 | 9G/1F | 1 | sole-straggler: base+16 trilemma, 10 spellings |
| playerCheckIfClimbingOntoWall | dlls/objects/195_Player/player | 3348 | 99.791 | 5 | 3 | 15G/5F | 22 | flags360 mask family: li/and vs rlwinm — see LEAD 1 |
| debugTextDrawToFrameBuffer | main/dll_80136a40 | 384 | 97.656 | 6 | 2 | 7G/0F | 4 | priced 15 order bucket: param-home mr slid into the mulli gap; COLOURING |
| mmFreeDeferred | main/mm | 220 | 99.455 | 6 | 0 | 2G/0F | 2 | mm dead-operand cap (old worklist) |
| playerState25 | dlls/objects/195_Player/player | 1384 | 99.884 | 7 | 0 | 4G/4F | 22 | player-cluster coloring walls |
| playerStopRidingObject | dlls/objects/195_Player/player | 356 | 97.978 | 7 | 4 | 2G/0F | 22 | flags360 mask family: li/and vs rlwinm — see LEAD 1 |
| ObjSeq_onMapSetup | dlls/engine/2/2 | 760 | 99.211 | 10 | 2 | 4G/0F | 5 | surplus-queue W6/W7 + gLightmapDrawQueue struct wall |
| playerStateAttack | dlls/objects/195_Player/player | 2836 | 99.908 | 11 | 0 | 5G/2F | 22 | player-cluster coloring walls |
| boneParticleEffect_update | dlls/engine/24/24 | 1764 | 99.649 | 11 | 9 | 15G/6F | 1 | value-home-r0 / base-mat LO-fold wall (+ engine/24 pool 2b) |
| objDrawShadowCasterMesh | main/shadow_dolphin | 1132 | 99.509 | 12 | 1 | 7G/2F | 2 | shadow copy-survival cited (structscan doc) |
| gameTextFinalizeLoad | main/textrender_run | 1592 | 99.334 | 14 | 2 | 11G/0F | 4 | priced 15 order bucket: one slid instruction inside a register perm; COLOURING |
| GameUI_release | dlls/engine/0/0 | 336 | 98.929 | 14 | 0 | 7G/0F | 14 | engine0-hud-walls cluster (store-forward rule + copy-survival + recolour) |
| pauseMenuDrawStatusPage | dlls/engine/0/0 | 2692 | 99.851 | 19 | 0 | 8G/5F | 14 | engine0-hud-walls cluster (store-forward rule + copy-survival + recolour) |
| pauseMenuDrawGridCell | dlls/engine/0/0 | 1012 | 99.506 | 22 | 0 | 12G/7F | 14 | engine0-hud-walls cluster (store-forward rule + copy-survival + recolour) |
| drawArwingHud | dlls/engine/0/0 | 1064 | 99.436 | 27 | 0 | 10G/1F | 14 | engine0-hud-walls cluster (store-forward rule + copy-survival + recolour) |
| hudDrawCommunicatorAlert | dlls/engine/0/0 | 632 | 99.051 | 27 | 0 | 8G/5F | 14 | engine0-hud-walls cluster (store-forward rule + copy-survival + recolour) |
| textureLoad | main/texture | 1932 | 98.882 | 33 | 22 | 17G/0F | 3 | block-placement: dead unreachable trampolines / dead inline preheaders |
| pauseMenuDrawStatus | dlls/engine/0/0 | 2064 | 99.641 | 34 | 0 | 6G/2F | 14 | engine0-hud-walls cluster (store-forward rule + copy-survival + recolour) |
| unloadMap | main/shader | 616 | 97.922 | 43 | 1 | 10G/0F | 8 | surplus-queue W6/W7 + gLightmapDrawQueue struct wall |
| mapScreenDrawHud | dlls/engine/0/0 | 3456 | 99.647 | 47 | 1 | 12G/5F | 14 | engine0-hud-walls cluster (store-forward rule + copy-survival + recolour) |
| headDisplayDraw | dlls/engine/0/0 | 1920 | 98.802 | 47 | 4 | 12G/5F | 14 | engine0-hud-walls cluster (store-forward rule + copy-survival + recolour) |
| playerStateMoving | dlls/objects/195_Player/player | 4880 | 99.746 | 51 | 0 | 4G/3F | 22 | player-cluster coloring walls |
| cMenuSetItems | dlls/engine/0/0 | 1208 | 98.758 | 60 | 0 | 10G/0F | 14 | engine0-hud-walls cluster (store-forward rule + copy-survival + recolour) |
| expgfxGetSlot | dlls/engine/10_expgfx/expgfx | 792 | 95.899 | 63 | 4 | 4G/0F | 4 | expgfx-addregroup + unroll wall |
| hudDrawButtons | dlls/engine/0/0 | 3684 | 99.194 | 69 | 4 | 13G/0F | 14 | engine0-hud-walls cluster (store-forward rule + copy-survival + recolour) |
| hudDrawMagicBar | dlls/engine/0/0 | 2472 | 99.288 | 75 | 0 | 12G/0F | 14 | engine0-hud-walls cluster (store-forward rule + copy-survival + recolour) |
| mapLoadUnloadObjects | main/shader | 1912 | 98.372 | 112 | 21 | 13G/0F | 8 | surplus-queue W6/W7 + gLightmapDrawQueue struct wall |
| drawViewFinderHud | dlls/engine/0/0 | 4980 | 99.345 | 126 | 84 | 7G/18F | 14 | engine0-hud-walls cluster (store-forward rule + copy-survival + recolour) |
| modelRenderInterpolateRootTransform | main/render | 2212 | 96.682 | 140 | 117 | 17G/1F | 1 | 16-18-wide multi-defect (old worklist frontier) |
| Checkpoint_buildControlPoints | dlls/engine/3/3 | 2500 | 98.464 | 268 | 2 | 12G/12F | 1 | priced 15 order bucket: LICM preheader hoist order + param-home perm; COLOURING |

### hand-asm (5)

Not compiler output at all (custom ABI hand asm) or needs an instruction MWCC has no intrinsic for.

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | recorded mechanism |
|---|---|---|---|---|---|---|---|---|
| setGQR6 | main/model | 8 | 50.000 | 1 | 2 | 0G/0F | 8 | priced 5: no mtgqr intrinsic |
| setGQR7 | main/model | 8 | 70.000 | 1 | 1 | 0G/0F | 8 | priced 5: no mtgqr intrinsic |
| videoInit | main/pi_videoinit | 2132 | 99.512 | 6 | 4 | 2G/0F | 1 | pi-videoinit wall: mfmsr/mthid0, no intrinsic |
| modelBoneTransforms_next | main/model | 72 | 10.833 | 16 | 22 | 0G/0F | 8 | sole-straggler: custom calling convention, not C |
| modelApplyBoneTransform | main/model | 464 | 10.784 | 107 | 127 | 18G/0F | 8 | priced 5 never-touch island (PS/private ABI) |

### foreign-compiler (1)

Wrong compiler family entirely (ProDG).

| fn | unit | size | fuzzy | ndiff | struc | band | #nm | recorded mechanism |
|---|---|---|---|---|---|---|---|---|
| zlbDecompress | main/zlb | 2352 | 53.531 | 567 | 520 | 18G/0F | 1 | zlb ProDG, capped ~53-55 |
