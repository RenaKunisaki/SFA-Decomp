# Priced residual classes — the banned-fix ledger

Classes of residual divergence whose only known cure is a construct banned by the hack purge
(see `docs/HACK_AUDIT.md`), or a compiler/toolchain behaviour with no source-level lever at all.
Every row below is BANKED: the mechanism is root-caused and the listed probes were measured.
Do not re-probe a row without a genuinely new lever; when a new function shows a class tell,
bank it here on sight instead of re-deriving.

Scores are per-function `fuzzy_match_percent` at tip `d3addebce3` (2026-08-01) unless dated.
The "price" of a class = the score a row gives up because the historical fix is banned
(or, for compiler-side classes, because no fix exists).

## 1. Target-unmerged dot-compare (purge-priced)

**Mechanism.** Our `-O4,p` peephole record-merges a shift/rotate with the following compare
across the statement boundary, emitting a dotted form (`rlwinm.`/`rlwimi.`/`srwi.` setting cr0
directly). Retail's compile of the same code did not merge: it keeps the plain shift plus a
separate `cmpwi`/`cmplwi`. Historical fix was `#pragma peephole off` around the function — banned.

**Tell.** LEN-1 (or LEN-0 with one substitution) rows where target shows `shift + cmpw/cmplwi`
and ours shows a single dotted `rlwinm./rlwimi./srwi.` feeding the same branch.

**Rows and standing price (gap to 100):**

| Function | Unit | Score | Gap |
|---|---|---|---|
| atan2fHighPrecision | main/acosf | 99.125 | 0.875 |
| atan2f | main/acosf | 98.456 | 1.544 |
| __kernel_cos | main/acosf | 98.250 | 1.750 |
| removeButtonObject (unroll-guard `srwi.`+`cmplwi`) | main/gameloop_buttonobj | 98.091 | 1.909 |
| mathSinCosf (family cousin: surplus target `fmr f1,f28` O0 arg copy-back) | main/sincosf | 98.750 | 1.250 |

Probed dead ends: switch-on-expression loses the r31 web; splitting via `|=` flips the base
instruction; five spellings of the sincosf copy-back are inert. Un-banning `#pragma peephole off`
for exactly these functions would be expected to recover roughly the summed 7.33 fn-points
(tree-level impact well under 0.01%).

## 2. DLL literal-pool provenance: @-anon vs lbl_ pool (purge-priced)

Retail DLL-origin TUs place f32/f64 literals differently from what our per-TU compile can mint.
Two verified sub-shapes (both re-verified at tip; note A57's original record claimed the
DBstealerwo target object "has no .sdata2 at all" — that is the TRICKY target; DBstealerwo's
target has its own 0xE8-byte .sdata2):

- **(a) Cross-TU shared pool.** `dlls/objects/196_Tricky/tricky.c`: the target object has NO
  .sdata2 section; its relocs name `lbl_803E23xx/24xx` literals living in an sdata2 region that
  `splits.txt` assigns to no unit (a shared pool gap). Our TU mints local `@NNN` anons instead.
  The only organic spelling that reproduces the reloc is a banned `extern const` `lbl_` load.
  Row: trickyUpdateCircling 99.983 (T==C sequence-identical; residual is reloc-only).
- **(b) Own-pool mint-order rotation.** The target TU's own .sdata2 holds the same values in a
  different order than our mint (mint order is compiler-internal: proven immune to source
  branch swaps, and not statement-lowering order — measure, don't model). Reconstructing the
  order via `lbl_`-named consts is banned. Rows: dbstealerworm_update 100.0 but unit
  matched_data 1232/1464 (578_DBstealerwo, first two pool words swapped); objects/332 unit
  matched_data 168/236; engine/24 unit matched_data 1904/1936 (0.0f rotated around the three
  named drift consts).

**Tell.** `ndiff.py` shows ONLY `RELOC lbl_803Exxxx` vs `RELOC @NNN` replace rows on an
otherwise sequence-identical function, and/or unit matched_data short by a rotated .sdata2.

**Scoring facts (proven by experiment, use freely):** pool `lbl_`-vs-`@NNN` NAMING is not
scored at an equal address (#70; asinf is 100.0 with a fully permuted, all-named pool);
what scores is the different address/offset. Pool mint order is immune to source branch swaps.
The legal named-const reconnect — `const f32 name[1] = {v};` with `[0]` uses (scalar const
folds to an anon; extern-decl+def lands .sdata, wrong section) — is exactly score- and
matched_data-neutral (24.c `gBoneParticleDriftMax/Rebound/Min`, commit `d3addebce3`); it may
only be used when the naming law is satisfied, and it does NOT recover the rotation bytes.

## 3. Peephole-gated unroll base-chunking (purge-priced)

**Mechanism.** In an unrolled copy loop retail folds the stride into the load/store
displacements of up to 32 copies off ONE base register; with peephole ON our compile re-bases
(surplus `addi base,base,stride` per copy, chunk 8). Controller isolated to `-opt nopeephole`:
retail's TU had peephole off AND chunked at 32 — unreachable from source (25 source shapes
swept on the walkgroup row; `lwz`-fed opaque bases fold but wreck the prologue, 95.832).
TU-wide flag path CLOSED: flipping the Hcurves unit's flags costs the rest of the unit far
more than the row recovers (unit fn-score sum 6598.9 -> 5941.0 measured at B21 time).

**Rows:** Objfsa_UpdateWalkGroupPatches (Hcurves, ex-`walkgroupFindExitPointFn_800dc398`)
99.459; SaveGame_gplaySetObjGroupStatus (engine/23) 97.981 (retail folds the 3-byte stride of
the 4x-unrolled transient-bit scan into `lbz` displacements; 3 rewrites probed inert).

**Tell.** Target: single `mr`/base + arithmetic-progression displacements across unroll copies.
Ours: identical base plus one surplus `addi` per copy.

## 4. Large-constant-HI never CSE'd across a call (compiler-side; GC/2.0 AND GC/1.3)

**Mechanism.** MWCC never CSEs the `lis` half of a large constant across a call. Retail hoists
`lis rSAVED,K` once and emits `addi rD,rSAVED,lo` at each site; we re-materialise `lis;addi`
per site. Probe-proven with a minimal harness (plain literal, propagated locals, 3-use,
differing-LO, 0x1ffffff variants all re-materialise). The only hoisting spelling is a rolled
2-trip loop, but MWCC will not unroll a loop containing a call — in-tree that craters the row
(TryCastSpell 98.714 -> 94.44). No fix exists, banned or otherwise.

**Rows (the 0x200001 spawnObject quartet, player.c):** playerState30 99.173,
playerStateShootFireball 98.905, playerStateTryCastSpell 98.714, playerStateAimStaff 99.320.

**Tell.** Target `lis rS,K` into a saved reg + N x `addi rD,rS,lo` spanning `bl`/`bctrl`;
ours repeats the pair per site.

## 4b. Retail-deleted redundant ext before a narrow store in a nopeephole TU (compiler-side)

**Mechanism.** The narrow-store extension rule (lane C9) is upheld: at `-opt nopeephole`,
`s16field += <int-typed expr>` always emits `extsh` before the `sth`. Retail's
`babyCloudRunner_turnTowardTarget` (objects/332, nopeephole TU proven by its siblings) shows
that ext deleted while every other instruction — including the surrounding
extend-at-use pattern — matches our nopeephole compile byte-for-byte. Recompiling the same
source with `-opt peephole` reproduces retail's store region exactly, but full peephole also
transforms the rest of the function (fn 98.86->93.52 on the TU flip; render 100->91.25,
sequenceCallback 100->94.22). Retail's pipeline applied the redundant-ext deletion without the
other peephole transforms — a pass subset no flag or (banned) pragma selects. No fix exists.

**Probe evidence (A59).** 34-case spelling fuzz + targeted probes over local type x callee
return type x statement structure x cast placement x lhs form (bitfield/union/pointer): the only
source-level elision paths are (a) an unpromoted-narrow-typed rhs per the C9 matrix, and
(b) a NEW deferred form — `s16 y = s16call();` leaves `y` unextended, a statement-level
`y >>= k` then shifts the RAW register and `field += y` elides — but under `-O4,p` (GC/2.0 and
GC/1.3.2) the shifted value is rematerialised (`srawi`+`extsh`) at every later use, never
materialised once (measured in-tree: 98.86 -> 89.20). GC/1.1/1.2.5 canonicalise eagerly
instead. The retail single-`srawi` + extend-at-use shape is unreachable from source; the
refcorpus has ZERO instances of `lha; srawi; add; sth`-without-ext across all 38,736
functions x 4 profiles.

**Rows:** babyCloudRunner_turnTowardTarget (objects/332) 98.864, gap 1.136. Sibling tell
banked by C9: dll_98_func03 (152) `sth`->`lha` store-forward weld 99.77.

**Tell.** A nopeephole unit whose ONLY diff is a surplus `extsh`/`extsb`/`clrlwi` immediately
before a narrow store, with the rhs int-typed and extend-at-use everywhere else matching.

## 5. Toolchain caps (source-unreachable; sight-list)

Closed classes with no reachable spelling — bank on sight, details in the memory topic files:

- **#108 GPR reg-perm** (dominant residual): T==C length register permutations are WELDED,
  incl. r0-vs-r3 and the fcmpu operand pair; mixed-kind perms inert.
- **#82 FP-perm** at the same load count — uncontrollable except the sdata2-literal->plain
  crack (saved FPRs) and the cross-branch launder.
- **#67 frame/displacement**: objdiff normalises r1 displacements, frame immediates, and one
  surplus base-`addi` — score-free.
- **#70 reloc-name-vs-@NNN at an EQUAL address**: score-neutral, but blocks a Matching flip;
  prove with `objdump -s` identical bytes.
- Zero-weld `li`-vs-`mr`; dead-tail `b`-stubs; allocator remat (incl. unroll-tail-bound remat —
  respelling the bound flips the whole unroll shape, -4); invariant-address CSE weld;
  same-field-reload (our CSE forwards where retail reloads a field it also names — but see the
  asymmetry law: a raw SECOND read of a reloaded DISTINCT field is load-bearing);
  peephole branch-FOLD; array-subscript value-numbering; flow-sensitive const-prop;
  large-const misc (#110, #113, #126).
- **Foreign islands (never touch):** model.c modelApplyBoneTransform +
  modelBoneTransforms_next (private ABI), ObjModel_Transform* PS-asm bodies (the 3 permanently
  unscored functions), zlbDecompress, pi_videoinit, render.o gap_03; fn_80007F78 (register
  pressure).

## 6. Banned-shape removal in a demoted DLL TU (purge-priced, measured 2026-08-02)

The `tools/banned_shapes_check.py` sweep of `21b90aff9f` + `5b120c0545` removed 22 pool
anchors / volatile puns and 2 gotos across 15 DLL units, demoting each. Both commit messages
record `dfuzzy +0.000000 / ddata +0.000000 / 0 per-function regressions`; a full rebuild plus
`report.json` at each of the two shas measures otherwise, so the rows below are the real
standing price. That false zero is what `tools/score_delta_gate.py` now exists to prevent: the
purge DEMOTED every unit it touched, and a demoted unit is invisible to the forced-link/DOL gate,
while `matched_code` does not move when a unit's literal pool shrinks. Only a scan of per-function
`fuzzy_match_percent` **and** per-unit `matched_data` **and** the complete flags, over full
rebuilds at both endpoints, can see this class. The gate reproduces the rows below from the two
shas, and refuses to print a zero until a synthetic regression injected into real source has come
back RED.

Window `cd782d6179` -> `5b120c0545`: tree 99.81533 -> 99.81422, matched_data 1198129 ->
1197137 (-992), complete_units 877 -> 862. Every row below was 100.0 before the purge.

| Function | Unit | Score | Gap | Shape removed |
|---|---|---|---|---|
| worldobj_spawnGreatFoxEffects | objects/467 | 98.333 | 1.667 | `static const f32 s...[1] = {0.64f}` |
| renderClouds | engine/9 | 99.429 | 0.571 | `volatile f32 gCloudActionGlareQuadSize[2]` |
| mmpMoonRock_update | objects/386_MMP_moonroc | 99.516 | 0.484 | inner-loop early-exit `goto` |
| trickyBallMove | objects/245_SidekickBal | 99.637 | 0.363 | cross-arm `goto noFloorDepth` |
| SmallBasket_spawnContents | objects/260_SmallBasket | 99.908 | 0.092 | `const f32 g...[1]` x4 |

**Mechanisms, one per row.**

- **467.** The anchor's `[0]` read is a load from a user object, so it stays a source-placed
  statement and is emitted before the loop-invariant array-base hoist. A plain `0.64f` is
  const-folded, its load becomes an optimizer-placed invariant remat, and it lands AFTER
  `lis/addi gGreatFoxEffects` instead of before it. That 2-instruction move is the entire
  divergence; the pool itself still mints in retail's order. Probed inert or worse: hoisted
  assignment statement, initialiser-at-declaration, literal inlined with the local deleted,
  `while` form, swapped comma-init order, function-scope `effect`, all 24 local-decl
  permutations (all 98.333), explicit base-pointer local (96.667), block-scope `effect`
  initialiser (96.575).
- **engine/9.** Retail issues two back-to-back `lfs` of `gCloudActionGlareQuadSize` for the
  third quad vertex; without `volatile` our build value-numbers them into one. No non-volatile
  spelling forces a reload of a plain global that the compiler has already proven unchanged.
- **MMP_moonroc.** Seeding `spacingClear = 1` before the loop makes it live ACROSS the loop,
  costing one extra callee-saved GPR (`_savegpr_26` vs retail `_savegpr_27`); the `goto` wrote
  the flag on both exit paths and needed none. Probed: assign-on-both-paths 98.606,
  `spacingClear = i == count` after the loop 99.240, zero-seed + post-loop test 98.813 — the
  landed seed-and-break form is already the best structured spelling.
- **SidekickBal.** The `goto` tail-shared the else arm's `li r0,0`; the structured form
  duplicates it. Probed: hoisted zero-init 98.924, merged-condition else 97.128, ternary
  98.045 — again the landed form is the best.
- **SmallBasket.** #82 scratch-FPR perm (`fdivs f2,f31,f30` vs ours `f1`) that the anchor had
  pinned. Four spellings of the health-percent chain probed: two inert, two worse.

**Data price.** The anchors were real `.sdata2` atoms, so deleting them shortens and rotates
each unit's pool: 209_TumbleWeedB -148, 260_SmallBasket -120, 678_ARWSquadron -120,
213_Kaldachom -112, 701 -92, 300_Transporter -80, 373_DFropenode -76, 523_FireFly -76,
679_ARWProximit -64, 683_LGTProjecte -32. Not recoverable without the anchors; the
class-2 note applies (the legal `const f32 name[1]` reconnect needs a satisfied naming law,
and none of these symbols has one).

The `203 -72` row that stood here is RECOVERED and struck (`a21f332847`, 2026-08-02): that
unit's anchor was not deleted, it was demoted to a `const f32` SCALAR, and MWCC folds a read
of a const scalar into a fresh pool literal. The named object was emitted at its declaration
point and then never referenced, while the two reads minted a second copy of `0.1f` at the
end of the pool. Restoring the one-element array form — legal here because
`gDllCBDefaultAnimSpeed` is in `config/GSAE01/symbols.txt` — reconnects the reads, drops the
duplicate, and returns 203 to 328/328 data. **Scalar-vs-array is the tell to check before
pricing any row in this section: a folded scalar shows up as a named `.sdata2` object with
zero relocations against it and a duplicate of its value elsewhere in the pool.**

**Standing verdict.** All five rows are at their best structured spelling; do not re-probe
without a genuinely new lever. The DOL still holds because every affected unit was demoted.

## 6b. Same class in two `src/main` units: `trig` + `rcp_dolphin` (purge-priced, measured 2026-08-02)

`4461d0aa45` removed eight `const T x[1] = {V}` anchors — five sin/cos approximation
coefficients from `src/main/trig.c`, three distortion constants from `src/main/rcp_dolphin.c`,
demoting `rcp_dolphin`. The commit measured and stated `matched_data -272` (trig -192,
rcp_dolphin -80) at `matched_code +0` and `matched_functions +0`. All three of those figures
reproduce exactly. They are also the half of the price that a threshold counter can see:
`matched_code` and `matched_functions` only move when a function crosses 100.0, and every
`trig` function was already at 99.97-99.98, so eight of them lost fuzzy score without moving
either counter by a byte.

| Function | Before | After | Gap |
|---|---|---|---|
| fsin16HighPrecision | 99.981 | 94.212 | 5.769 |
| fcos16HighPrecision | 99.981 | 94.212 | 5.769 |
| fsin16Precise | 99.976 | 95.585 | 4.390 |
| fcos16Precise | 99.976 | 95.585 | 4.390 |
| fcos16 | 99.973 | 96.730 | 3.243 |
| fsin16 | 99.973 | 96.730 | 3.243 |
| fsin16Approx | 99.970 | 98.152 | 1.818 |
| fcos16Approx | 99.970 | 98.152 | 1.818 |

Tree fuzzy 99.81535 -> 99.811676, **-0.003674 — 3.3x the entire 22-instance section-6 window
(-0.00111) from a quarter as many instances.** `trig` `.text` 99.975 -> 95.926 and its `.sdata2`
100.0 -> 61.458 at unchanged section size, with unit `matched_data` 192 -> 0: removing five words
rotates the whole pool, so the section loses every byte rather than the five. `rcp_dolphin` keeps
`.text` at 100.0 and pays only `.sdata2` 100.0 -> 14.634 (-80), and its demotion is what keeps the
DOL green.

**Standing verdict.** Banked, not reverted — section 6's law applies: the shape is banned and the
anchors are load-bearing, so there is no free subset to retune toward. The only known recovery is
section 2's legal `const f32 name[1] = {v}` reconnect, which is gated on the naming law and on the
checker's cross-TU exemption; that is a pool/naming-lane question, not a retune. Reproduce with
`python3 tools/score_delta_gate.py --commits fff7ee912c 86334e8343`.

## Related recurring REGRESSION class (fixable — not priced, listed so windows get scanned)

Pool-const purge/retune commits historically gated on matched_code only and twice shipped
per-fn regressions (fcmpo operand-order flips: sky2_run via `zero < best.x` -> `best.x > zero`,
fixed in `2c32711828`; DIMCannon_updateAim sibling) and once shipped matched_data -4936
(engine/0 purge, retuned in `f596800ffa`). Delta-scan every merge window with per-fn fuzzy
AND matched_data; an fcmpo flip is repaired by restoring the retail operand order in source.
The `21b90aff9f`/`5b120c0545` pair (section 6) is the third instance and the first where the
regression was NOT retunable — a purge whose price is real still has to be measured and
banked, not reported as zero.

## 7. Uncalled statics are NOT automatically fabricated (class REFUTED, 2026-08-02)

A census of our-only `.text` symbols (present in `build/GSAE01/src/**.o`, absent from the
retail carve) reports ~2.4 KB across 18 game units, and the shape is genuinely dangerous:
objdiff pairs our functions against RETAIL functions by name, so a body that is not in the
DOL has no pair and is never scored. It is therefore a free place to park fabricated code
that exists only to mint `.sdata2` literals in the order the carve wants. Two separate
classes hide under that one census number, and only one of them is a hack.

**Not the class — a plain static WITH call sites (820 B, 14 units).** MWCC inlines a plain
`static` at every call site and *still* emits the out-of-line body; mwld strips it at link.
`WM_LevelCon`, `LargeCrate`, `Landed_Arwi`, `502`, `DIMCannon`, `ARWSpeedStr`, `obj_movelib`,
`329`, `engine/73`, `446`, `501` and `WCTempleBri` are all this. It is the deliberate idiom
of `caf9ee4472` (WCTempleBri), where compiling the deform helper at its definition site is
what puts retail's bias doubles in the right order. Leave them.

**The real question — a static with NO call site (1616 B, 8 units).** Decided by one
experiment: *MWCC does not intern a file-scope const against a pool literal.* Declaring
`const f32 k[1] = {4.0f};` in `main/curves.c` alongside functions that already load `4.0f`
grew `.sdata2` from 0x44 to 0x64 — nine duplicated words. So a slot that live retail code
loads can never have been a declared const; it is a pool literal, minted on FIRST USE in text
order. Whenever such a slot sits AHEAD of the first live function that loads it, the only
possible minter is code that ran earlier and is not in the DOL — dead code that mwld stripped.

Every phantom-minted slot in all eight units is loaded by a live retail function:

| unit | uncalled statics | phantom-minted slots | shared with live code | delete cost |
|---|---|---|---|---|
| `main/curves.c` | 4 | 9 | 9 | -68 |
| `599_DR_EarthWar` | 2 | 14 | 14 | -192 |
| `625` | 2 | 6 | 6 | -104 |
| `700_Andross` | 1 | 9 | 9 | -268 |
| `main/shadow_dolphin.c` | 1 | 2 | 2 | -88 |
| `engine/20_Hcurves` | 1 | 1 | 1 | -72 |
| `Hcurves_romcurve` | 1 | 1 | 1 | -88 |
| `203` | 1 | 0 | 0 | 0 |

The sharpest single proofs: `curves.c` slot 0x18 holds `0.16666667f`, which no surviving
function computes with, yet it sits eighth in a pool whose ninth slot is the first live
function's first literal; and `599_DR_EarthWar` puts the compiler's own int-to-float bias
double at 0x08, ahead of `DR_EarthWarrior_feed`'s `4.32f` at 0x10 — a bias double cannot be
declared, so something converted an int before the first surviving function ran. Its second
uncalled helper accounts for slots 0x38-0x48 as `0.02, 2.0, 0.5, 0.75, 32768.0`, the exact
literal order of a five-statement body. **Deleting these seven reconstructions would cost 880
matched_data and buy nothing; they stay, and they are in the checker baseline.**

`203`'s `dll_CB_getStateHandler` was the one true positive: no call site, no minted slot, not
one data byte moved. Deleted in `a21f332847`.

**Gate.** `tools/banned_shapes_check.py` now carries `UNCALLED_STATIC_FN` — a source-only,
transitive census (a cluster reachable only from other uncalled statics is itself uncalled,
which is how `curveSpeedAt` is caught). `static inline` is out of class: an inline nothing
calls is never expanded and never emitted. A new hit is not automatically a hack — adjudicate
it against the unit's pool with the sharing test above before accepting or deleting it.
