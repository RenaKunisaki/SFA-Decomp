# Band-width worklist — where a structural fix can actually stick

`tools/bandscreen.py` ranks every function that differs from retail by **saved-register
band width**, and splits the difference into:

- **structB** — instructions whose *mnemonic* differs: a real shape difference a source
  edit can address.
- **regB** — same mnemonic, different operands: band/allocation assignment.

**Rank by `structB`.** That is the source-addressable part.

## Why band width is the right axis

The register-band assignment model is exact for narrow bands and falls off a cliff as the
band fills. Measured over 6,288 SFA retail functions, scoring the load-class rule in
CLAUDE.md (non-copy saved regs = one declaration-keyed population filling `r31`/`f31`
downward), stratified by total saved regs in the band:

| saved regs in band | 2 | 3 | 4 | 5 | 6+ |
|---|---|---|---|---|---|
| GPR | **97.7%** | **99.3%** | **98.8%** | 1.4% | 0.1% |
| FP  | 45.5% | 36.8% | 19.3% | 14.9% | 5.1% |

It is a cliff, not a decay — **the break is at 4->5, so the screen defaults to
`--max-band 4`**, not 3. FP is much weaker at every width, consistent with Lane A's
third (`sqrtf`, assignment-keyed) population being common in FP-heavy code — trust a
probe over the rule there.

Every frontier function that resisted a whole campaign is wide-band —
`expgfx_updateActivePools` 18G/10F, `modelRenderInterpolateRootTransform` 17G,
`mapLoadDataFile` 10G, `collectShadowTrackTriangles` 10G/2F, `allocLotsOfTextures`
8G/16F, `blendTextures` 8G, `renderObjects` 6G. Both functions flipped to 100% in a
single session were narrow-band: `drawFn_8006f500` (2G/2F) and `mmFreeDeferred` (2G).
Four exhaustive declaration sweeps (720 / 225 / 144 / 121 candidates) returned *zero*
movement, and every one was a wide-band function.

## The important caveat: narrow band means PREDICTABLE, not STEERABLE

A narrow band means the assignment follows the rule reliably — so if the source has the
right shape you get the right registers, and a probe result transfers. It does **not**
mean a declaration edit can move the registers.

Measured: `playerUpdate` is 3G/1F (the 98.1% regime) and its 576 `regB` are a clean
`r29`<->`r30` swap between an incoming parameter and a value loaded from it. A **full
120-permutation sweep of its five local declarations was completely flat**, as were four
argument-order spellings against its 28 `structB`. So `regB` in a narrow band is still
allocation, and still not reachable by declaration order.

Use the worklist to find **structB**; treat regB as diagnosis, not as a target.

## Using it

    python3 tools/bandscreen.py --struct-only    # the worklist below
    python3 tools/bandscreen.py                  # narrow band, incl. regB-only
    python3 tools/bandscreen.py --max-band 2     # only the 99.8% regime
    python3 tools/bandscreen.py --all            # every differing function

`structB`/`regB` are differing instructions x4 — a ranking proxy, not `report.json`
truth. Always gate a real change on `python3 tools/unitfuzzy.py <unit>`.

Once this file has told you **where**, `docs/source_shape_levers.md` covers **what to write** —
the levers that landed, each with the measurements that refuted it elsewhere. Note that this
ranking has **no memory**: it will happily rank a documented cap as the largest unworked entry,
so `grep` the memory files and `git log` the function before starting.

## Caveats

- Interior `lbl_*` symbols in retail objects are folded into the preceding function.
  Without that, objdump compares a 3-instruction fragment of retail against a whole
  function of ours; that produced a spurious top entry (`PSMTXMultVecArray`, a unit that
  is in fact 100%). If you add a new symbol convention, re-check this fold.
- A function can be narrow-band and still capped for a reason already on record —
  `mmFreeTick`'s dead strength-reduction IV, `loadTextureFiles`'s orphaned inline-return
  preheaders, `hitDetectFn_800658a4`'s FP-abs shape, `trig.c` (toolchain wall, CLOSED).
  Check the memory notes before re-attacking a listed entry.
- `main/textrender_boxtex.c` has an active owner; coordinate before touching it.
- **`main/model.c` entries are structurally blocked** (retail hand-written asm) and the
  screen cannot see it. `modelBoneTransforms_next` ranks #1 by structB (60/72) purely
  because retail's version passes results in callee-saved registers (`r20`/`r21`/`r15`)
  without saving them — a fragment sharing register state with its caller. Same for the
  `zlb` foreign-GCC region. Skip both.
- **`main/pi_videoinit.c videoInit` (16 structB) is asm-bound — do not chase it.** Retail
  emits `mfmsr/ori/mtmsr` + `mfhid0/ori/mthid0` inline in the epilogue where we emit
  `bl PPCMfmsr` ... `bl PPCMthid0`. The source is already correct
  (`PPCMtmsr(PPCMfmsr() | MSR_PM)`). Making those SDK helpers inline is not the answer:
  `OSCache`, `OSThread` and `PPCArch` all call them and are **100%**, so retail did not
  inline them globally — it used inline asm in this TU only, which CLAUDE.md bans in
  `src/main/`. Correctly left NonMatching.
- `structB`/`regB` are computed by aligning the two streams on their **mnemonic**
  sequence, so a whole-function register rename stays out of the structural bucket.
  Three earlier versions of this classifier each produced a false #1; if you change it,
  re-run `differing()` against a known rename (an `r5`<->`r7` swap must give
  `struct=0`).
- `sizeB` is the retail function size; where ours differs in length the two disagree.

## Current worklist (band <=4, ranked by structB)

```
# frontier ranked by band width -- <=4 saved regs in BOTH bands
# 37 of 657 differing functions qualify; structB 324/6056, regB 3500/60952

# structB = instructions whose MNEMONIC differs (source-addressable shape);
# regB    = same mnemonic, different operands (band/allocation)

structB  regB     sizeB    G    F     unit                               function
60       52       72       4    0     main/model.c                       modelBoneTransforms_next
16       244      792      4    0     dlls/engine/10_expgfx/expgfx.c     expgfxGetSlot
16       72       1500     4    0     main/lightmap.c                    sceneDraw
16       60       1916     0    0     dolphin/MSL_C/PPCEABI/bare/H/exponentialsf.c powf
16       32       264      1    2     main/trig.c                        fsin16Approx
16       8        2132     2    0     main/pi_videoinit.c                videoInit
12       8        964      4    1     dlls/objects/195_Player/player.c   playerStateTryCastSpell
12       0        436      1    0     main/texture.c                     loadTextureFiles
12       0        512      2    3     dlls/objects/195_Player/player.c   fn_802AA2B0
8        580      2372     3    1     dlls/objects/195_Player/player.c   playerUpdate
8        212      560      0    0     musyx/runtime/synth_seq_dispatch.c seqInit
8        136      788      3    0     main/mm.c                          mmFreeTick
8        64       608      2    0     main/lightmap.c                    updateEnvironment
8        56       2584     4    2     dlls/objects/195_Player/player.c   fn_802B0EA4
8        28       280      2    0     main/audio.c                       streamsLoadedCallback
8        24       648      2    0     dlls/objects/597/597.c             SnowBike_UpdateRouteFollowing
8        24       1384     4    4     dlls/objects/195_Player/player.c   playerState25
8        20       220      0    0     main/gameloop_buttonobj.c          removeButtonObject
4        556      936      3    0     main/textrender_boxtex.c           gameTextInitBoxTextures
4        512      760      4    0     dlls/engine/2/2.c                  ObjSeq_onMapSetup
4        208      4880     4    3     dlls/objects/195_Player/player.c   playerStateMoving
4        180      632      2    0     main/pi_pathsearch.c               pathSearchBegin
4        112      840      3    1     dlls/objects/655_WCPressureS/WCPressureS.c wcpressures_update
4        108      356      4    0     main/texture.c                     textureInitGXTexObj
4        56       352      4    0     dlls/objects/332/332.c             babyCloudRunner_turnTowardTarget
4        48       1900     4    0     dlls/objects/196_Tricky/tricky.c   trickyDigTunnel
4        32       524      3    0     dlls/objects/589_BossDrakor/BossDrakor.c bossdrakor_updateHeadTracking
4        24       2324     3    0     main/shader.c                      beginLoadingMap
4        20       148      0    4     dolphin/MSL_C/PPCEABI/bare/H/math_8029454c.c mathTanf
4        8        316      2    1     main/track_dolphin.c               trackGetNearestGroundOffsetAndNormal
```
