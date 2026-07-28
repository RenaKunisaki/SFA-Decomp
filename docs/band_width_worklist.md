# Band-width worklist — where a structural fix can actually stick

`tools/bandscreen.py` ranks every function that differs from retail by **saved-register
band width**, and flags whether the difference is in the *mnemonic stream* (STRUCT —
a real shape difference a source edit can address) or only in the *operands* (REG —
allocation noise).

## Why band width is the right axis

The register-band assignment model is near-exact for narrow bands and collapses as the
band fills. Measured over 6,288 SFA retail functions, scoring the joint model
(materialized values ascending + computed values descending, simultaneously):

| saved regs in band | 2 | 3 | 4 | 5 | 6+ |
|---|---|---|---|---|---|
| GPR | **99.8%** | **98.1%** | 86.0% | 48.1% | 27.0% |
| FP  | 86.5% | 63.8% | 55.9% | 37.5% | 25.6% |

At <=3 saved registers the band is effectively deterministic, so **structure is the only
free variable** and a probe result transfers into the real function. At >=6 there is no
total order to steer: four exhaustive declaration sweeps (720 / 225 / 144 / 121
candidates) returned *zero* movement, and every one was a wide-band function.

The correlation is what makes this actionable. Every frontier function that resisted a
whole campaign is wide-band — `expgfx_updateActivePools` 18G/10F,
`modelRenderInterpolateRootTransform` 17G, `mapLoadDataFile` 10G,
`collectShadowTrackTriangles` 10G/2F, `allocLotsOfTextures` 8G/16F, `blendTextures` 8G,
`renderObjects` 6G. Both functions flipped to 100% in one session were narrow-band:
`drawFn_8006f500` (2G/2F) and `mmFreeDeferred` (2G).

## Using it

    python3 tools/bandscreen.py --struct-only    # the worklist below
    python3 tools/bandscreen.py                  # narrow band, incl. REG-only
    python3 tools/bandscreen.py --max-band 2     # only the 99.8% regime
    python3 tools/bandscreen.py --all            # every differing function

`diffB` is differing instructions x4 — a ranking proxy, not `report.json` truth. Always
gate a real change on `python3 tools/unitfuzzy.py <unit>`.

## Caveats

- **REG-classified functions are not necessarily hopeless**, they are just not
  *structurally* addressable; they need the scratch-band flag analysis instead
  (`tools/fn_flag_probe.py`).
- A function can be narrow-band and still capped for an unrelated reason already on
  record (`mmFreeTick`'s dead strength-reduction IV, `loadTextureFiles`'s orphaned
  inline-return preheaders, `hitDetectFn_800658a4`'s FP-abs shape). Check the memory
  notes before re-attacking a listed entry.
- `sizeB` is the retail function size; where ours differs in length the two will not
  agree.

## Current worklist (narrow band AND structural)

```
# frontier ranked by band width -- <=3 saved regs in BOTH bands
# 25 of 657 differing functions qualify; 2400 of 67024 diff-bytes

# of those, 2400 diff-bytes are STRUCT (mnemonic stream differs -- source-addressable); the rest are REG (allocation noise)

diffB    sizeB    G    F    kind    unit                               function
604      2372     3    1    STRUCT  dlls/objects/195_Player/player.c   playerUpdate
560      936      3    0    STRUCT  main/textrender_boxtex.c           gameTextInitFn_8001c794
220      560      0    0    STRUCT  musyx/runtime/synth_seq_dispatch.c seqInit
184      632      2    0    STRUCT  main/pi_pathsearch.c               pathSearchBegin
144      788      3    0    STRUCT  main/mm.c                          mmFreeTick
128      12       0    0    STRUCT  dolphin/mtx/mtxvec.c               PSMTXMultVecArray
116      840      3    1    STRUCT  dlls/objects/655_WCPressureS/WCPressureS.c wcpressures_update
72       608      2    0    STRUCT  main/lightmap.c                    updateEnvironment
64       1916     0    0    STRUCT  dolphin/MSL_C/PPCEABI/bare/H/exponentialsf.c powf
56       264      1    2    STRUCT  main/trig.c                        fsin16Approx
36       280      2    0    STRUCT  main/audio.c                       streamsLoadedCallback
36       524      3    0    STRUCT  dlls/objects/589_BossDrakor/BossDrakor.c bossdrakor_updateHeadTracking
28       220      0    0    STRUCT  main/gameloop_buttonobj.c          removeButtonObject
28       2324     3    0    STRUCT  main/shader.c                      beginLoadingMap
28       648      2    0    STRUCT  dlls/objects/597/597.c             SnowBike_UpdateRouteFollowing
24       2132     2    0    STRUCT  main/pi_videoinit.c                videoInit
12       436      1    0    STRUCT  main/texture.c                     loadTextureFiles
12       316      2    1    STRUCT  main/track_dolphin.c               trackGetNearestGroundOffsetAndNormal
12       260      1    1    STRUCT  main/track_dolphin.c               hitDetectFn_800658a4
12       512      2    3    STRUCT  dlls/objects/195_Player/player.c   fn_802AA2B0
8        176      2    0    STRUCT  main/model.c                       modelGetAmapSize
4        8        0    0    STRUCT  main/model.c                       setGQR6
4        8        0    0    STRUCT  main/model.c                       setGQR7
4        24       0    0    STRUCT  dolphin/os/OSTime.c                OSGetTime
4        252      0    0    STRUCT  musyx/runtime/voice_id.c           vidMakeNew
```
