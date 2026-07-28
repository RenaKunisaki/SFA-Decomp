# Band-width worklist — where a structural fix can actually stick

`tools/bandscreen.py` ranks every function that differs from retail by **saved-register
band width**, and splits the difference into:

- **structB** — instructions whose *mnemonic* differs: a real shape difference a source
  edit can address.
- **regB** — same mnemonic, different operands: band/allocation assignment.

**Rank by `structB`.** That is the source-addressable part.

## Why band width is the right axis

The register-band assignment model is near-exact for narrow bands and collapses as the
band fills. Measured over 6,288 SFA retail functions, scoring the joint model
(materialized values ascending + computed values descending, simultaneously):

| saved regs in band | 2 | 3 | 4 | 5 | 6+ |
|---|---|---|---|---|---|
| GPR | **99.8%** | **98.1%** | 86.0% | 48.1% | 27.0% |
| FP  | 86.5% | 63.8% | 55.9% | 37.5% | 25.6% |

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
- `sizeB` is the retail function size; where ours differs in length the two disagree.

## Current worklist (narrow band, ranked by structB)

```
# frontier ranked by band width -- <=3 saved regs in BOTH bands
# 24 of 657 differing functions qualify; structB 196/6844, regB 1636/50988

# structB = instructions whose MNEMONIC differs (source-addressable shape);
# regB    = same mnemonic, different operands (band/allocation)

structB  regB     sizeB    G    F     unit                               function
28       576      2372     3    1     dlls/objects/195_Player/player.c   playerUpdate
24       32       264      1    2     main/trig.c                        fsin16Approx
16       8        2132     2    0     main/pi_videoinit.c                videoInit
12       0        436      1    0     main/texture.c                     loadTextureFiles
12       0        512      2    3     dlls/objects/195_Player/player.c   fn_802AA2B0
8        64       608      2    0     main/lightmap.c                    updateEnvironment
8        56       1916     0    0     dolphin/MSL_C/PPCEABI/bare/H/exponentialsf.c powf
8        20       648      2    0     dlls/objects/597/597.c             SnowBike_UpdateRouteFollowing
8        0        280      2    0     main/audio.c                       streamsLoadedCallback
8        0        220      0    0     main/gameloop_buttonobj.c          removeButtonObject
8        0        788      3    0     main/mm.c                          mmFreeTick
8        0        560      0    0     musyx/runtime/synth_seq_dispatch.c seqInit
4        556      936      3    0     main/textrender_boxtex.c           gameTextInitFn_8001c794
4        180      632      2    0     main/pi_pathsearch.c               pathSearchBegin
4        112      840      3    1     dlls/objects/655_WCPressureS/WCPressureS.c wcpressures_update
4        32       524      3    0     dlls/objects/589_BossDrakor/BossDrakor.c bossdrakor_updateHeadTracking
4        0        176      2    0     main/model.c                       modelGetAmapSize
4        0        8        0    0     main/model.c                       setGQR6
4        0        8        0    0     main/model.c                       setGQR7
4        0        2324     3    0     main/shader.c                      beginLoadingMap
4        0        316      2    1     main/track_dolphin.c               trackGetNearestGroundOffsetAndNormal
4        0        260      1    1     main/track_dolphin.c               hitDetectFn_800658a4
4        0        24       0    0     dolphin/os/OSTime.c                OSGetTime
4        0        252      0    0     musyx/runtime/voice_id.c           vidMakeNew
```
