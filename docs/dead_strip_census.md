# The dead-stripped-code census (measured 2026-08-03)

Code that our translation units emit and the retail link discarded is the one corner of the tree
no score gate can see. `objdiff` pairs our functions against retail functions **by name**, so a body
that is not in `main.dol` has no pair and is never scored; `matched_code` never counts it, the
forced link never notices it, and `obj_equal` compares our objects only with each other. That makes
it a place where fabricated code could sit forever. This file is the census, and
`tools/dead_strip_census.py` re-runs it.

## Population

Comparing `.text` sizes over the 1005 unit pairs (`build/GSAE01/obj` carve vs `build/GSAE01/src`):

| | count |
|---|---|
| unit pairs whose `.text` size differs | 71 |
| ...where OURS is larger | 56 (12 408 B) |
| ...where the CARVE is larger | 15 |
| units carrying whole functions only we have | **49** |
| such functions | **86 (12 348 B)** |

The 15 carve-larger rows are not this class: 2 are unsplit gap symbols (`render.o gap_03`,
`targimpl.o gap_03`) and the other 13 are 4–28 B body-size differences inside already-sub-100 units.

## The three mechanisms, which are not interchangeable

**The surplus is NOT one phenomenon, and the majority of it is not dead code at all.**

| class | mechanism | count | bytes |
|---|---|---|---|
| **A** | **INLINED-AND-STRIPPED** — live code calls it, MWCC inlined every call site and emitted the out-of-line copy anyway, mwld stripped the copy | **50** | **7 332** |
| **B** | **UNCALLED STATIC** — nothing calls it, not even transitively (`UNCALLED_STATIC_FN`) | 27 | 2 228 |
| **C** | **UNREFERENCED GLOBAL** — not static, nothing references it, mwld stripped it | 9 | 2 788 |

Class A is 58% of the population and it is **the normal fate of a small static helper at `-O4`**.
`446.c`'s `lavaball1be_applyDebrisGravity` is the type specimen: it is called at line 167 of its own
file, our object carries its body at `.text+0`, and **our object contains no `bl` to it anywhere** —
every call was inlined. Retail's compiler did the same, which is exactly WHY the carve's `.text`
starts 0x2c later. Reading a `.text` surplus as evidence of invented code is therefore wrong for
most of this population, and a non-transitive call scan gets it wrong in the other direction too:
`curves.c`'s four helpers form a chain (`curveBuildArcSegments` → `curveSolveArcParam` →
`curveIntegrateSpeed` → `curveSpeedAt`) in which only the head is unreferenced, so the inner three
look "called" unless liveness is iterated to a fixpoint. Class B must be taken from
`banned_shapes_check.scan_uncalled_statics`, which does iterate.

## The proof, which is positive rather than an absence of evidence

`dead_strip_census.py excise` removes exactly the stripped functions' byte ranges from our `.text`
and compares what is left with the carve's `.text`.

**34 of the 49 units reproduce the carve BYTE FOR BYTE.** That proves two things at once: the
stripped bodies are the whole of the difference, and every surviving instruction — including the
**inlined copies of the class-A helpers** — is identical to retail's. A fabricated helper would have
to inline into exactly retail's instruction stream. Nothing in those 34 units' `.text` is invented.

All 34 are at `fuzzy 100.0` with `complete: true`; all 15 that fail are below 100.0 and fail on an
ordinary residual elsewhere in the unit, which says nothing about their dead-stripped code either
way. The negative control matters: shifting every excision range by 4 bytes keeps the resulting
length right and the content wrong, and drops the pass count — so the test is not merely comparing
sizes. It is in the self-test.

## Fabrications found: none

The one true-positive shape is **a body that mints nothing and moves no data byte** — `203`'s
`dll_CB_getStateHandler`, deleted in `a21f332847` (see `priced_classes.md` §7). Disassembling every
class-B and class-C body over its own byte range:

**0 of the 36 carry zero relocations.** Every one mints pool literals (`@NNN`) and/or names real
data or callees. None is dead weight. **Nothing was deleted, and nothing should be.**

## What the evidence is, per class

- **A (50 fns)** — the excision byte-match, above. Positive and complete for the 100.0 units.
- **B (27 fns)** — 26 are already adjudicated in `tools/banned_shapes_baseline.txt` under the
  pool-sharing test of `priced_classes.md` §7. The 27th, `synth_jobs.c streamGainFromVolume`, has
  never been adjudicated **because `src/musyx` is an `EXEMPT_ROOT`** — the checker cannot see it.
  Its unit is 100.0/complete and passes excision, and it mints `@2` and `@4`, so it meets the same
  standard the other 26 met.
- **C (9 fns)** — `src/dolphin/os/OSExec.c` is a **line-for-line correspondent of an independent
  GameCube decompilation**: `reference_projects/super_mario_strikers/src/Dolphin/os/OSExec.c` has
  the same 18 functions in the same order, offset by one line. `__OSFPRInit` appears in that same
  project's `OS.c`, as `asm` on both sides. Genuine Nintendo SDK, faithful, keep.

## Genuinely undecidable: 1 unit

`musyx/runtime/synth_seq_queue.c` (0x194 B, `synthInitChannelEventQueues` and
`synthRefreshChannelEventQueue`). Its carve `.text` is **0** — the whole unit is absent from the
DOL — so the excision test compares 0 bytes with 0 bytes and has **no power at all**. Its callees
`GenerateNextTrackEvent` and `InsertGlobalEvent` are real MusyX names, corroborated in the symbol
tables of metroid_prime, marioparty4, super_mario_strikers and super_smash_brothers_melee; but the
two function names themselves appear in **no** independent decomp, and the only reference project
that carries them is an older copy of **this project**, which is circular. Undecidable, and it
should be left alone rather than guessed at in either direction.

`dolphin/os/OSExec.c` is the same shape — carve `.text` 0, no instrument power — but it is decided
anyway, by the reference decomp above. That is the difference between "no gate sees it" and
"nothing can be known about it".

## Two instrument defects this census exposed

1. **`banned_shapes_check`'s `UNCALLED_STATIC_FN` is static-only and skips `src/dolphin` and
   `src/musyx`.** Both restrictions are correct for what that check is for, but the consequence is
   that **10 dead functions totalling 2 818 B (the 9 class-C globals plus `streamGainFromVolume`)
   are screened by nothing in the tree.** `dead_strip_census.py` is what covers them.

2. **Five units report `fuzzy 100.0` and `complete: true` with no code, no data and no functions**
   — `dolphin/ax/AX`, `dolphin/TRK_MINNOW_DOLPHIN/MWCriticalSection_gc`, `dolphin/os/OSExec`,
   `musyx/runtime/synth_sequence`, `musyx/runtime/synth_seq_queue`. Their `total_code`,
   `total_data` and `total_functions` are all absent from `report.json`. **So 5 of the 915
   `complete_units` are vacuous**, and a 100.0 on those five rows is not a statement about
   correctness. Two of them (`OSExec`, `synth_seq_queue`) are exactly the units whose whole `.text`
   was dead-stripped, which is why the vacuity and the census meet here.

## What this closes, and what it does not

It closes the question the surplus `.text` raised: **no fabricated function is hiding there.**
It does not make the class self-policing — a future lane can still add an uncalled static, and only
`banned_shapes_check` (inside `SCAN_ROOTS`) or this tool (everywhere) will report it. Run
`dead_strip_census.py census` when a unit's `.text` grows without its score moving.
