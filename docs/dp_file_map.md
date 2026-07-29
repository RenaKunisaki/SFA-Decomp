# Dinosaur Planet -> Star Fox Adventures file correspondence

```
Lane C39, derived on origin/staging @ 0eaa172f02.

HOW THIS WAS DERIVED (so the next lane can re-run / extend it)
-------------------------------------------------------------
Four independent lenses, all reproducible; scripts in /private/tmp/c39/.
  L1  FUNCTION-SYMBOL VOCABULARY.  `readelf -sW` every built SFA source .o for FUNC symbols
      (/private/tmp/c39/sfa_fns.txt), grep every DP src/*.c for definitions
      (/private/tmp/c39/dp_fns.txt), split both name sets into camelCase/underscore tokens and
      score each (SFA unit, DP file) pair by IDF-weighted token overlap, BOTH directions.
      => /private/tmp/c39/fnmap.py.  This is the lens that produced most rows below.
  L2  PREFIX HISTOGRAM.  Per file, the histogram of function-name prefixes (DP is rigorously
      prefixed: map*/block*/cam*/vox*/font*/fbfx*/route*/joy*/rcp*/obj*).  Catches whole-module
      moves that L1's IDF dilutes.
  L3  STRING-LITERAL INTERSECTION between DP src and SFA src.  Weak (SFA's decomp carries far
      fewer literals) but ZERO false positives: curves/objhits/object/objanim/model/voxmap/
      memory all self-pair, and map.c<->shader.c and intersect.c<->track_dolphin.c both fire.
  L4  BODY READ.  Used to settle every row marked [BODY] below.

*** CRITICAL CAVEAT, verified this lane ***
  L1 is NOT circular: SFA and DP share almost no function names (the raw name intersection over
  all 58 SFA units x 57 DP files is 8 symbols).  SFA's names were NOT bulk-imported from DP.
  BUT DP's own names ARE derivations - DP's source says so in its own comments
  ("// official name: setBlendMove (probably)", "// official name: trackGetIntersect2 ?").
  USE DP FOR SEMANTICS.  For NAMES, the only DP evidence that outranks our own body reading is
  DP's *printf string literals*, which are the original's words exactly as retail's are:
    "trackIntersect: FUNC OVERFLOW %d\n"   (present verbatim in BOTH DP and SFA retail)
    "trackIntersect: linefunctable overflow!!!\n"
    "trackIntersect() MAX_LINEPOINTS overflow!!!\n"
    "WARNING: trackGetLoaded bit overflow\n"
    "trackGetIntersect2: Bad ABC (not reporting any more)\n"
    "<objanim.c -- setBlendMove> WARNING tried to load anim -1 from cache modno %d\n"  (BOTH)
  Anything else in DP is a fellow decompiler's guess and must be re-earned from our body.

*** SECOND CAVEAT: SFA's src/main file NAMES are OURS, not retail's. ***
  The `.c` strings visible in the target .o files are dtk's STT_FILE symbols generated from our
  own split config, NOT retail data.  The only source file names retail itself utters are:
    objanim.c, objHitReact.c, expgfx.c, Hcurves.c, camcontrol.c, textblock.c, laser.c,
    n_attractmode.c, DIMBoss.c, SHthorntail.c   (plus the SDK's dvdfs.c / musyx snd3d*.c)
  Note objhits.o carries retail's "objHitReact.c: sphere overflow! %d" -> retail's own name for
  that TU's hit-reaction half is objHitReact.c, and `objhits.c` is a DP-derived label.
  => Do NOT treat a src/main filename as evidence for anything.  Several are actively
  misleading; see the src/track row below.

THE TABLE
---------
Legend:  [BODY] settled by reading both bodies.  [L1] top-1 by vocabulary, large margin.
         [STR] corroborated by a shared string literal.  [?] plausible, NOT settled.

DP file                SFA unit(s)                              evidence / notes
---------------------- ---------------------------------------- --------------------------------
map.c            (193) main/shader.c  +  main/tex_dolphin.c      [L1 161 / 124, huge margin][STR]
                       +  main/lightmap.c  +  lightmap_draw.c    DP's ONE map module is split
                       +  main/lightmap_initmapblocks.c          four ways in the GC tree.
    -> shader.c            = DP's `map*` half: rom-list/layout/load/unload/warp/texture-scroll
                             (DP mapReadLayout/mapSaveObject/mapWarpPlayer/mapFree/mapIsLoaded/
                              blockTexscroll*/blockTexanim*/blockColorTable* ...)
    -> tex_dolphin.c       = DP's `block*` half: block emplace/load/render (DP blockLoad,
                             blockEmplace, blockFree; SFA MapBlock_loadFromFile/MapBlock_init/
                             mapBlockRender_*)
    -> lightmap*.c         = DP's `blockAddToRenderList` + `dl*` display-list/scene half
                             (SFA updateVisibleGeometry / renderSceneGeometry / sceneDraw)
    Named pairs worth lifting: mapWorldXZToMapID~mapCoordsToId, mapAreWorldCoordsInMap~isInBounds,
    mapWorldXZToBlockIndex~mapGetBlockIdx, mapWarpPlayer~warpToMap, mapFree~unloadMap,
    blockTexscrollSet/Get/Tick~mapTextureScrollSetStep/GetOffset/Acquire.
    C38's anchor holds: DP mapInitObjSetupList == SFA mapBuildRomListIndex, and DP
    gMapObjSetupLists is 0x41A0 exactly as SFA's per-map record table.

intersect.c       (56) main/track_dolphin.c                      [BODY, DECISIVE][L1 23][STR]
    DP intersect.c and SFA track_dolphin.c are the SAME module: both define a 21-function `track*`
    API.  DP: trackGetHeight{,Ceiling,Floor,Nearest} trackGetIntersect trackGetLineIntersect
    trackIntersect{Init,Tick,Broadphase,BuildAABB,GetBlockList,GetPLIndices,MarkBlocksDirty,
    ModLineBuild,NeedsUpdate,Initialized,LastLineTick} trackToggleHitLine.
    SFA already carries the retail-attested `trackIntersect` (it prints
    "trackIntersect: FUNC OVERFLOW %d") and `intersectModLineBuild`.
    ** Highest-value unlifted pair: DP `trackGetLineIntersect(Vec3f*,Vec3f*,f32,s32,
       TrackLineIntersectResult*,Object*,s8,s8,u8,s8)` is SFA's `objBboxFn_800640cc` -
       TEN parameters, identical types in identical order, and the SFA body does exactly what
       DP's does (sweep a circle from `from` to `to` against every group-6 object's local-space
       lines, then against the static track).  See "HANDED OVER" below.
    Other pairs: trackIntersectNeedsUpdate~trackIntersectRebuildPending,
    trackIntersectGetBlockList~trackGetBlockDescriptors,
    trackIntersectMarkBlocksDirty~trackInvalidateDynamicSlotsForObject,
    trackToggleHitLine~trackSetLinesEnabledByParam,
    trackIntersectTick~trackTickDynamicSlotCooldowns, trackIntersectBuildAABB~(SFA hitDetectFn_*).

camera.c          (90) main/camera.c                             [L1 132, dominant]
object.c          (81) main/object.c                             [L1 108][STR]
objanim.c         (15) main/objanim.c                            [L1 64][STR][retail: objanim.c]
objhits.c         (90) main/objhits.c                            [STR][retail: objHitReact.c]
    L1 cannot see this pair: 87 of DP objhits.c's 90 functions are still `func_8xxxxxxx`.
    DP objhits.c is therefore a CONSUMER of our names, not a source of them.
objmsg.c           (6) main/objhits.c  (folded in)               [BODY, EXACT 6<->6]
    DP objSendMesg / objSendMesgMany / objSendMesgManyNearby / objPeekMesg / objRecvMesg /
    objMesgQueue  ==  SFA ObjMsg_SendToObject / ObjMsg_SendToObjects /
    ObjMsg_SendToNearbyObjects / ObjMsg_Peek / ObjMsg_Pop / ObjMsg_AllocQueue.
model.c           (39) main/model.c  (+ main/modelEngine.c)      [L1 38][STR]
lighting.c        (16) main/modellight.c                         [?] L1 is diluted because SFA
    renamed the whole module to `modelLightStruct_*`; DP's lightSetAmbient / lightSetDimAmbient /
    lightInit / lightBlockSphereMapping / lightModelSphereMapping / lightUseSkyLight are the
    role vocabulary to check our generic `lightSetFieldBC_*` against.
texture.c         (19) main/texture.c  (+ main/tex_dolphin.c)    [L1]
rcp.c             (21) main/rcp_dolphin.c + main/intersect_screenmath.c  [L1 17 / 9]
    plus the `Rcp_*` functions that ended up in shader.c and texture.c.  DP rcpClearScreen /
    rcpDrawPauseScreenFreezeFrame / rcpInit / rcpScreenColour / rcpBorderColour.
di_rcp.c           (5) main/rcp_dolphin.c                        [?]
framebuffer_fx.c  (51) track/intersect_render.c                  [BODY]
    DP fbfxBurnPaper / fbfxSineWaves / fbfxWeirdResizeCopy / fbfxDoEffect / fbfxPlay / fbfxTick
    == SFA doHeatEffect / renderMotionBlur / doBlurFilter / doColorFilter / doDistortionFilter /
    doSpiritVisionFilter / drawSnowFlashOverlay / renderWhirlpool.  Same module, GC rewrite.
objprint.c        (14) main/objprint_dolphin.c                   [L1 21][name]
    DP objprintDrawObject / DrawModel / DrawChildModel / DrawShadowModel / BlendColor /
    MultiplierColor / MatrixOverride / LockIconCoords.
newshadows.c      (33) main/newshadows.c  (+ main/shadow_dolphin.c)  [name][L1 12 on shadow_dolphin]
    DP prefixes the whole module `shadows*`; SFA's is generically named (`get*`, `fill*`), so DP
    is a genuine NAMING lead here and it is UNWORKED.
shadowtex.c        (6) main/shadow_dolphin.c                     [?] DP shadowtexInit / exDraw /
    exSwapBuffer ~ SFA shadowInit / shadowBeginFrame / shadowVolumeBeginFrame.
voxmap.c          (42) main/voxmaps.c                            [L1][STR][name]
generic_queue.c   (10) main/voxmaps.c  (folded in)               [BODY] SFA Queue_Init/Push/Pop/
generic_stack.c   (11) main/voxmaps.c  (folded in)               Peek/IsEmpty/GetCount + Stack_*
route.c           (18) main/voxmaps.c (voxmaps_*Route*)          [BODY] DP routeFind/routeHeapInsert/
                       + main/pi_pathsearch.c                    routeUpHeap/routeDownHeap/
    routeScanNeighbors/routeIsGoal/routeNext/routeAddPoint/routeAddNeighbor/routeClear/routeCurve
    == SFA voxmaps_updateRoutePath / _processRouteQueue / _expandRouteNeighbors /
       _visitRouteNeighbor / _getRouteNode / _buildRouteWaypoints / _allocRouteWork
    and, for the second A* instance, pathSearchBegin / pathSearchStep / pathSearchExpandNode /
       pathSearchHeapSiftDown / pathSearchNodeMatchesTarget / pathSearchBuildPath.
memory.c          (24) main/mm.c                                 [L1 35][STR][name: both use `mm`]
curves.c          (19) main/curves.c  + dlls/engine/20_Hcurves   [L1 28][STR]
    retail's own string is "Hcurves.c: MAX_ROMCURVES exceeded!!" - the DLL half is Hcurves.c.
joypad.c          (27) main/pad.c                                [L1 16]
vi.c              (20) main/pi_videoinit.c (+ pi_dolphin.c)      [L1 16]
pi.c               (7) main/pi_dolphin.c                         [L1][name: SFA kept the `pi` tag]
asset.c           (19) main/gameloop.c + main/pi_dolphin.c       [BODY] DP assetLoadAnim/LoadModel/
    LoadObject/LoadDLL/EnqueueLoad/QueueTick/RomLoad ~ SFA animationLoad / loadAsset /
    loadAssetFileById / loadTextureFile / getTabEntry / doQueuedLoads / loadDataFiles.
acache.c           (4) main/gameloop.c  (cacheAllocAndCopy)      [?]
rarezip.c          (6) main/zlb.c                                [BODY] the Rare zip/ZLB decompressor
                                                                 (SFA's is FOREIGN-COMPILER, do not touch)
print.c           (26) main/dll_80136a40.c                       [BODY] DP diPrintf/sprintf/strcpy
di_cpu.c          (17) main/dll_80136a40.c                       [L1 14]  == SFA debugPrintf /
di_comm.c          (9) main/dll_80136a40.c                       debugPrintfxy / debugPrintDraw /
di_cpu_stack.c     (1) (none - N64 debug stack walker)           debugPrintDrawGlyph / logPrintf /
debug.c            (3) main/dll_80136a40.c                       errDisplayHandler / errorThreadFunc.
fonts.c           (29) main/textrender.c + textrender_run.c      [L1 11][BODY]
                       + textrender_drawbox.c + gametext*.c      DP fontWindow{XY,Draw,UseFont,
    EnableWordwrap,DisableWordwrap} / fontRenderText / fontRenderFillRect / fontStringFormat /
    fontWordwrap / fontYSpacing / fontExtraCharSpacing / fontSquash / fontBgColour
    == SFA gameTextSetWindow / textRenderStr / gameTextMeasureString / gameTextRenderStrs / ...
    retail's own name for the DLL-side text block is textblock.c ("<textblock.c Init>").
main.c            (34) main/gameloop_main.c + main/gameloop.c    [L1 22][BODY] DP mainLoop etc.
                       + main/modelEngine.c (the game timer)     == SFA gameLoop / gameUpdate /
    main / getGameState / setGameState / checkReset / cutsceneEnterExit / blankScreen.
dll.c             (12) main/modelEngine.c (partial)              [?] DP dllLoad/dllFree/dllInit/
    dllLoadFromTab/dllReplaceLoadedDLLs/dllThrowFault.  SFA's GC port moved most DLL loading into
    the OSLink path; only the UI-DLL slice (getDLL16 / loadUiDll / setCurUiDll / uiDll_run*) and
    Resource_Acquire/Release survive in modelEngine.c.  NOT SETTLED - see OPEN ROWS.
objexpr.c         (28) dlls/engine/10_expgfx/expgfx.c            [STR: retail says "expgfx.c:"]
objlib.c          (14) main/obj_movelib.c (partial)              [?] DP's touch-callback half
    (objInvokeTouchCallbacks / objRemoveTouchCallback{,sForObj}) is NOT in obj_movelib.c;
    only the space-transform half lines up.  NOT SETTLED - see OPEN ROWS.
objtype.c          (5) main/object.c (partial)                   [?] no `objType*` symbol survives.
footsteps.c       (10) (unlocated)                               OPEN ROW.  SFA has per-DLL
    `*_spawnFootstepEffects` and gDb*FootstepSfx data but no central footsteps module found.
menu.c            (27) main/thp/picmenu.c / n_options.c          [?] OPEN ROW.
lfx.c / envfx.c    (3) main/skystars.c / render.c(getEnvfxAct)   [?] SFA render.c literally has
    getEnvfxAct / getEnvfxActImmediately - DP's envfx.c is 2 functions.  Weak but suggestive.
scheduler.c       (20) (none)                                    N64 OS scheduler; GC uses OS/VI.
boot.c / reset.c   (9) main/boot_logo.c / gameloop_main checkReset  [?]
audio.c / mp3 /   (16) main/audio*.c                             NOT a correspondence: DP is the
  segment_BED0.c                                                 N64 audio driver, SFA is MusyX.
libultra/*             src/dolphin/*                             platform SDK, unrelated bodies.
bitstream.c / linked_list.c / generic_* / assert.c / mpeg.c / developer_names.c / rsp_segment.c /
segment_11EF0.c / segment_11F70.c / segment_13D0.c               no located SFA counterpart.

SFA UNITS WITH NO DP COUNTERPART (GameCube-only work)
-----------------------------------------------------
  main/shader_dolphin.c        the GX/TEV shader-stage builder (N64 had no TEV)
  track/intersect_memcard.c    GameCube memory-card save/load (N64 used the controller pak)
  track/intersect_mtx44.c, track/intersect_screenmath.c   GX matrix/screen helpers
  main/modelEngine.c (most), main/gametext*.c (localisation), main/thp/*  (THP movie player)
  main/acosf.c / trig.c / sincosf.c / vecmath.c   MSL/PS math
  src/musyx/**                 MusyX

*** THE src/track/ FILENAMES ARE MISLEADING - RECORDED, NOT ACTED ON ***
  `src/track/intersect*.c` contain NO track-intersection code.  intersect_render.c is the
  framebuffer-FX + HUD-draw + TEV-stage module (DP framebuffer_fx.c), intersect_memcard.c is the
  memory card, intersect_screenmath.c/intersect_mtx44.c are GX matrix helpers, intersect.c is
  water FX + surface SFX.  DP's intersect.c is SFA's main/track_dolphin.c.  Renaming five source
  FILES is zero-score, high-churn and collides with four live lanes; handing to the tree owner.

OPEN ROWS (state, do not guess)
-------------------------------
  1. DP dll.c  -> where did dllLoad/dllFree/dllReplaceLoadedDLLs land in SFA?
  2. DP objlib.c's touch-callback half (objInvokeTouchCallbacks & friends).
  3. DP footsteps.c and menu.c.
  4. DP objtype.c: SFA has no surviving `objType*`; the type index may be inside object.c.

HANDED OVER (analysis done here, the file belongs to another lane)
------------------------------------------------------------------
  `objBboxFn_800640cc` (src/main/track_dolphin.c, held by B32 body-only; called from player.c
  (A39/A40), tricky.c, 211, 423, LanternFire, WM_Galleon) is DP's `trackGetLineIntersect`:
  ten parameters, identical types in identical order, identical body role.  A two-sided rename
  is defensible on the BODY alone (it sweeps a circle of `radius` from `from` to `to`, first
  against every group-6 object in its own local space via trackSweepCircleAgainstLines, then
  against the static track) and it joins the file's existing trackIntersect /
  trackSweepCircleAgainstLines / intersectModLineBuild family.  C39 did not take it because the
  token substitution spans six DLL files owned by three live lanes.
```
