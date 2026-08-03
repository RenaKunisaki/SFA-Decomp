# Priced residual classes — the banned-fix ledger

Classes of residual divergence whose only known cure is a construct banned by the hack purge
(see `docs/HACK_AUDIT.md`), or a compiler/toolchain behaviour with no source-level lever at all.
Every row below is BANKED: the mechanism is root-caused and the listed probes were measured.
Do not re-probe a row without a genuinely new lever; when a new function shows a class tell,
bank it here on sight instead of re-deriving.

Scores are per-function `fuzzy_match_percent` at tip `d3addebce3` (2026-08-01) unless dated.
The "price" of a class = the score a row gives up because the historical fix is banned
(or, for compiler-side classes, because no fix exists).

## Index of the measured laws

Sections 8-13 outgrew the "banned fix" framing: they are the campaign's measured model of what
MWCC's front end puts where, and of what a declaration costs. This index is the entry point.
Each row states the law in one line and says where its measurement lives; nothing is restated
here that the target section already says.

| Law | Where | The measurement behind it |
|---|---|---|
| **The mover.** Only one file-scope construct puts a pool word ahead of its first live loader: a `static const` aggregate of at most 8 bytes. A plain literal and a `static const` scalar land at the use; an external-linkage `const` lands at the declaration *and* emits a duplicate that the use loads. | §12, table in §11 | `main/rcp_dolphin` `.sdata2` 14.63 -> 100, `279_AppleOnTree` 90.62 -> 100, `701` 95.65 -> 100; `matched_data` +300, `complete_units` 906 -> 909, every function byte-identical. The spelling is the banned `SINGLE_ELEM_CONST_ARRAY`, and §12b measures four spellings of it emitting the same object byte for byte, so **the ban is on the shape, not the bracket**. Not landed; patch parked at `/private/tmp/A68_declared_consts.patch`. |
| **An aggregate places only its FIRST word for free.** Every mover word therefore needs its own four-byte symbol. | §12c | `701` with `{0.0f, 1.0f}` comes out `.sdata2` byte-identical, but element `[1]` compiles to `li rN,0` + SDA21 + `lfs f,4(rN)` where retail has one `lfs f,0(r2)`. |
| **Mint order is the source TEXT's order, not the code's.** The front end interns at the *use*, before the code generator runs, so a function can sit at 100.0 with a rotated pool. Within one expression the order is increasing expression-tree depth; an 8-byte literal defers to the next 8-aligned offset and the 4-byte hole it leaves is back-filled by the next 4-byte literal interned — if one follows, and otherwise it stays a zero word. | §10, §11 | The value-sequence oracle (`tools/pool_value_sequence.py`) at `e173a2c951`: 22 of 33 sub-100 sections have every function's value sequence *identical* to retail's, so their code already asks for the same constants in the same order. `704` and `trig`'s `fsin16Approx` reproduce the depth rule exactly; `704`'s `titleScreenShowCopyright` reproduces the deferral and the back-fill. The unfilled case is the leading `00000000` §12's survey finds at the hole in front of a double. |
| **`.bss` is the same question asked for free.** Declaration order is completely inert; layout is first-use order, shallowest sub-expression first. | §11, "`.bss` order is a second, free oracle" | A 36-cell declaration x use matrix (6 declaration permutations x 6 use permutations of three `.bss` arrays) depends only on the use order. `main/objlib` is the specimen where `.text`, `.bss` and `.sdata2` disagree about the same file's source order. |
| **`.sdata2` scores all-or-nothing per section, so a partial fix is worth strictly less than zero.** Removing five of a section's words rotates the whole pool. | §6b, §8; `docs/source_shape_levers.md` "a section scores ALL-OR-NOTHING" | `4461d0aa45`: `trig` `.sdata2` 100.0 -> 61.458 at *unchanged section size*, unit `matched_data` 192 -> 0. |
| **A value can carry its own sign.** Retail stores `-C` in the pool and adds; spelling the same polynomial as `A - C` mints words retail never had and emits `fnmsubs` for `fmadds`. | §6b (standing verdict) | 44 sites in `src/main/trig.c` rewritten `A + -C`: tree 99.811966 -> 99.815640, `.text` 95.926 -> 99.975, `.sdata2` 61.458 -> 93.750, all eight sin/cos functions back to their pre-purge figures to the digit, 0 REGRESSED. An operator deleted, no shape added. |
| **Restore is not declare.** If the pool's value *multiset* already matches and only the positions are wrong, you are looking at a deleted function, not a missing constant - MWCC interns plain literals across a TU, so restoring a body mints nothing new. | `docs/purge_campaign_audit.md`, "The recovery law" | `997e72e3e1`: `tricky` differed in zero words and 101 of 102 positions, `player` in zero words and 8 of 196; restoring seven deleted bodies took `matched_data` +1192 with both pools byte-identical to the carve. Where the TU *also* carries a one-element anchor minting the same value, delete the anchor (`558c86a421`). |
| **Price the opaque-extern crutch PER SYMBOL, not per unit**, and use the oracle before assuming one: a never-defined extern is a crutch only if retail's own split object defines the symbol; a carve-blob symbol is faithful. | §9, §9b, §10b | The `nm` oracle over retail's split objects; `engine/5`'s lone missing word is the priced crutch - defining it buys +176 data and costs `renderSunAndMoon` 99.476 -> 98.214, a net tree loss. |
| **The defining TU proposes, the reading TU disposes**, and a declaration can only move into a header that can NAME the type - type visibility, not the include, is the constraint. Extern arrays stay UNSIZED. | §13 | See §13; every rule there was established md5-identical over all 1013 source objects. |
| **The gate blind spots, and why md5-of-every-`.o` dominates.** Demotion blinds the DOL gate; `matched_code`/`matched_functions` are threshold counters; a pool rotation inside an already-NonMatching unit is free on every score axis. | `docs/purge_campaign_audit.md`, "Three sensor blind spots"; the docstring of `tools/score_delta_gate.py` | `4461d0aa45` moved neither counter and still took a function 99.981 -> 94.212; `5d467157cb`/`f5fe00213f`/`620b69dc2d` lost 144/60/16 B at `dfuzzy +0.000000`, zero regressions, zero demotions. The pool word-diff catches the third class; md5 of every `.o` catches all four *and* the score-neutral ones (class #70 renumbering), which no score can see. |
| **The toolchain caps and the never-touch islands.** Bank on sight; do not re-probe. | §5 | Per-class detail in the memory topic files. |
| **Every sub-100 code row is one of three kinds, and the kinds are decidable without reading any source.** Same opcode sequence, different registers = colouring. Same opcode multiset, different sequence = order. Different multiset = a different operation, which only the source text chooses. | §14 | `tools/a71_mnhist_scan.py` + the partition in §14 over all 213 sub-100 rows at `97746b6bd3`: **136 colouring / 10 order / 67 operation**, no fourth kind and no reloc-only row. |
| **The order bucket is not a third kind.** Every "order" row is one (at most two) instructions out of place inside a register permutation, so open one only when the slid instruction is the one the source text names. | §15 | All 10 order rows of §14 worked one at a time: mnemonic-sequence delta is 1 for eight of them and 2 for the other two. One paid (`renderShadows` 99.69954 -> 99.71494, the two squares of a magnitude named in retail's order); the nine that did not slid a parameter home, a rematerialised constant, a LICM hoist or a scheduler's delay-slot filler. |
| **The carve's symbol table is an oracle no score reads.** `.bss` order is free evidence of the source's use order (§11), and a name that contradicts the carve at the same offset is a naming defect that every gate is blind to. | §16 | `tools/bss_order_scan.py` over all 1013 source objects: **21** `.bss` sections ordered differently than the carve and **109** name divergences. Exactly ONE object says `lbl_` where the carve has a recovered name — `597.c`'s `lbl_803E5AE0`, already `sSnowBikePathPointParams` in `config/GSAE01/symbols.txt`: renaming it is byte-identical across all five sections, deleting it costs `matched_data` −396 and `matched_code` −1364. |
| **`.bss` allocates at the first use that FOLLOWS the definition** — and an object with no use after its definition is allocated at end of TU, in reverse definition order. Refines the `.bss` row above: definition order is inert only among definitions that all precede their uses. **A NonMatching unit is linked from the carve, so its `.bss` order is invisible; a Matching one is linked from our object, and a permuted `.bss` moves every DOL word that names it.** | §17 | A six-case probe battery under the live flag sets, then all 21 mis-ordered sections closed by moving definitions past their last use: `bss_order_scan` 21 -> 0, section contents identical over all 1013 objects, `complete_units` 910 -> 914. |
| **The sign-in-the-constant class is EMPTY outside `trig`.** The `A + -C` recovery does not generalise; nothing else in the tree holds a value at retail's opposite sign. | §14 | `tools/a71_signscan.py` over all 1050 units: **0** units hold a mirror-signed float, and the opcode partition finds **0** `fnmsubs`/`fnmadds`/`fsubs`-for-`fmadds` rows outside the three never-touch PS islands. |

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
| atan2f_fast | main/acosf | 98.250 | 1.750 |
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
- **`setGQR6` / `setGQR7` — no `mtgqr` intrinsic (priced at 50.000, measured 2026-08-03).**
  Retail is two instructions, `mtgqr N,r3` and `blr`; MWCC GC/2.0 exposes no intrinsic for the
  GQR write and inline `asm{}` is banned in `src/main`, so the body can only be empty or a lie.
  `6b383c0b7f` deleted `setGQR6`'s write-only `sGQR6Config` shadow — correctly, it was fabricated
  `.sbss` — which lands the section at the carve size at `matched_data` +0 and takes the function
  **70.000 -> 50.000** (`main/main/model` `.text` 92.276 -> 92.270). That is the whole price and it
  is not recoverable: any two-instruction body still misses `mtgqr`, so it scores the same 70.000
  the fabrication did. `setGQR7` **stays** — the `ObjModel_Transform*Vertices*` scalar
  reconstructions genuinely read the quantisation exponent back out of `sGQR7Config`, and
  hard-coding a scale reaches the exact section size only by replacing a runtime value with a lie.
  A window that flags `setGQR6` is reading this row, not a new regression.

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

**Standing verdict — REFUTED for `trig` by `e173a2c951`, still standing for `rcp_dolphin`.**
The reading above assumed the anchors were load-bearing for the pool's ORDER. For `trig` they
were load-bearing for its VALUES: retail's `.sdata2` carries every Horner coefficient with the
sign already in the word (`bfc55555 5554c4d0`, `af9dd246`, `be927d83 c50b46da`), and the
post-purge source spelled the same polynomial as `A - C` with a positive `C`, so MWCC interned
eleven words retail never had and emitted `fnmsubs` where retail has `fmadds`. Writing `A + -C`
at the 44 sites — plain C, no shape added, an operator deleted — returns all eight functions to
the exact figures in the table above, `.text` 95.926 -> 99.975 and `.sdata2` 61.458 -> 93.750, and
the tree to 99.815640: `4461d0aa45`'s stated -0.003674 recovered to the digit, at 0 REGRESSED.
`rcp_dolphin` is a different problem — its value sequences already match retail's (section 10),
so its 14.634 is pure source-text order. Reproduce the original loss with
`python3 tools/score_delta_gate.py --commits fff7ee912c 86334e8343`.

The last 192 bytes of `trig` `.sdata2` are section 10's problem, not this one: retail interned
`fsin16Approx`'s cosine coefficients before its sine ones, and the case reorder that reproduces
it (reaching 100.0, +192) moves the emitted blocks and costs that function 98.152 -> 94.667.

## Related recurring REGRESSION class (fixable — not priced, listed so windows get scanned)

Pool-const purge/retune commits historically gated on matched_code only and twice shipped
per-fn regressions (fcmpo operand-order flips: sky2_run via `zero < best.x` -> `best.x > zero`,
fixed in `2c32711828`; DIMCannon_updateAim sibling) and once shipped matched_data -4936
(engine/0 purge, retuned in `f596800ffa`). Delta-scan every merge window with per-fn fuzzy
AND matched_data; an fcmpo flip is repaired by restoring the retail operand order in source.
The `21b90aff9f`/`5b120c0545` pair (section 6) is the third instance and the first where the
regression was NOT retunable — a purge whose price is real still has to be measured and
banked, not reported as zero.

**The scan for this class is `tools/invcmp_scan.py`, and the tell is DEFINITION ORDER, not a
mirrored register pair** (measured 2026-08-03). It is tempting to look for our stream carrying
retail's compare with its two operands swapped and the branch inverted; a tree-wide scan for
exactly that shape over all 211 sub-100 functions reports **zero** hits, and re-injecting the
historic `8c611c5062` mutation (`zero < best.x` -> `best.x > zero`, which cost sky2_run
99.653 -> 99.497) does **not** make it fire. The registers are not swapped at all. MWCC lists the
first-evaluated operand first, so flipping the source operands moves *which value is loaded
first*: retail emits `lfs f1,<zero>` then `lfs f2,20(r1)` and compares `fcmpo cr0,f2,f1`, while
the flipped source loads the live value first and compares against a zero it re-materialises.
What changes is the definition order of the two operands, which is what `invcmp_scan.py` already
measures — so run that, not a mnemonic-and-register differ.

At `79e394cfda` the scan reports three functions and no live row: `sky2_run`'s two `fcmpo` sites
(the pinned 99.65298 residual — retail defines the LEFT operand LAST at both, a shape
`invcmp_scan.py`'s own docstring attributes to a call or a call-ordered `static inline` on the
right-hand side, and the plain operand flip does not reach it), plus `modelApplyBoneTransform`
and `zlbDecompress`, which are §5 never-touch islands.

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

## Campaign-wide audit of the purge lane (2026-08-02)

`docs/purge_campaign_audit.md` rebuilds BOTH endpoints of all 42 purge-shaped commits and diffs
them with `tools/score_delta_gate.py`: 27 clean, 15 RED, gross -8168 B `matched_data` and
-0.032980 tree fuzzy, of which 5016 B were given back by the retunes already recorded here,
672 B by later pool work, and 2480 B still stand. Sections 6 and 6b cover 1192 B of that
residue. The remaining 1288 B is `72eec6655f`, which shipped with an empty commit body and is
priced nowhere; its open rows are listed in the audit and are recoverable.

The audit also adds a third sensor blind spot to the two this file already records: a pool
rotation inside an already-NonMatching unit loses `matched_data` at `dfuzzy +0.000000` with zero
per-function regressions and zero demotions, so neither the demotion tell nor the threshold
counters fire. `5d467157cb` -144, `f5fe00213f` -60, `620b69dc2d` -16.

## 8. `.sdata2` mint-order divergence with byte-identical code (measured 2026-08-02)

The residual `.sdata2` gaps left on the pool leads are one class, and it is not the
folded-scalar class of §7. Code is byte-identical; only the ORDER in which MWCC minted the
pool differs. Measured layout rule (this tree, GC/2.0):

1. `.sdata2` objects are laid out **sequentially in mint order**, 8-byte objects forced to
   8-alignment. The 4-byte hole that forcing leaves is **never backfilled** — 0 backfills
   across all 675 source objects that have a `.sdata2`.
2. Mint order is: file-scope objects at their **declaration point**, then per function in
   source order — its front-end literals in source order, then the backend's own
   int-to-float bias doubles.
3. A file-scope `const f32 X = V;` (`static` or not) is **folded at the read site**: the
   object is still emitted at its declaration point *and* a duplicate literal is minted at
   first use. Measured in `679_ARWProximit`: adding the three consts left all three duplicate
   literals in place, 18 words against retail's 16.
4. The only read that does not fold is an array subscript — i.e. the `SINGLE_ELEM_CONST_ARRAY`
   that `tools/banned_shapes_check.py` bans (75 baselined, regrowth is a gate failure).

**RETRACTION (same day).** The paragraph that stood here concluded that a displaced pool head is
unreachable because the only non-folding read is the banned 1-element pin. That is wrong, and
`015b98abbd` ("pool: recover the constants nine more units declare") had already refuted it.
The pin is adjudicated per instance, not forbidden outright: when the retail pool proves a slot
was a declared const, the instance is accepted into `tools/banned_shapes_baseline.txt` (75 -> 86
entries over that commit). Its reference-count spec is the adjudication test — a literal never
dedups into a file-scope const, so a slot's retail reference count says how many sites must
convert, and a value appearing twice in retail's pool but once in ours means the earlier copy is
a const and the later a literal. `679_ARWProximit`, listed below as priced, was recovered that
way. What survives from this section is the layout model above, the classifier in 8b, and the
three rows the const rule does not reach.

A second, hack-free route exists whenever the displaced values sit in a **duplicated block**
rather than in scattered scalars: extract that block as a `static` helper defined at the point
the pool wants the mint (the definition-site lens — `a4656c3766`, `144c1a9855`). Measured on
`679_ARWProximit` before the const version landed: two teardown helpers
(`arwproximit_destroyByHit` minting `0.0f, 100.0f`, then `arwproximit_detonate` minting
`127.0f`) placed ahead of `arwproximit_render` gave `.sdata2` 120/120 with all nine functions at
100.0, zero `banned_shapes_check` hits, and a clean `MatchingFor` flip (DOL sha1 unchanged).
Prefer it when the block is genuinely shared; it costs no baseline entry.

Sweep result: the §7 folded-scalar tell (a named `.sdata2` object with zero relocations) has
**17 instances tree-wide and every one is in a unit with `gap == 0`** — that lens is exhausted,
do not re-survey it.

| unit | gap | mechanism | probe result |
| --- | --- | --- | --- |
| `679_ARWProximit` | 64 | retail mints `0.0f, 100.0f, 127.0f` ahead of `arwproximit_render`'s `1.0f`; all three are read only from inside `arwproximit_update` | 1-element-array form gives **120/120 data, all 9 functions still 100.0** — and trips `banned_shapes_check` as regrowth. RECOVERED in `015b98abbd`; measured price before that was 64 B |
| `engine/68` | 128 | **not** a wrong constant: retail's `120.0f` at `.sdata2+0x44` is a plain literal of `firstPersonDoControls`, minted between `15360.0f` (0x40) and `16.0f` (0x48) | plain literal makes `.sdata2` byte-identical (64/192 -> **192/192**) but drops `firstPersonDoControls` 100.0 -> 94.512; tree 99.811676 -> 99.809730. PRICED 128 B |
| `engine/7` | 232 | one missing 4-byte mint cascades: retail mints a `1.0f` at 0x0c as a front-end literal of `lightningGetRemainingFraction`, after its `0.0f` and before its two bias doubles. Ours has only the `0.0f`, so 0x0c stays a hole, every later slot shifts 4, and a second hole opens at 0x84 | the missing `1.0f` emits no code in retail's `fn1` either — recovering it needs a phantom minter. DECLINE |
| `237`, `704`, `model`/`modellight`, `213_Kaldachom`, `279_AppleOnTree`, `597`, `195_Player`, `intersect_render`, `main/object` | 88-784 | same class; several heads are led by a bias double, which cannot be declared at all | not probed individually — the class verdict covers them |

`engine/68` carries a second, separate defect worth a code lane: `firstPersonDoControls` only
holds 100.0 because it divides by `gCameraModeViewfinderStickScale`, an `extern const f32` that
**no translation unit defines**. Being an opaque global, it blocks `-opt propagation` from
sinking the single-use temp `spinI` past the `camera->anim.rotX` store; with any in-TU form of
the constant MWCC sinks it and ~14 instructions move. The extern is a crutch, not a constant.
Also measured there: file-scope `const f32` 94.512; function-local `static const f32` folds to a
literal (data 192/192, code 94.512); `const f32 X[1]` restores 100.0 but the object lands at the
declaration point (0x00) or at the start of the function's static run (0x20), never at 0x44.

### 8b. The intra-function half of the class: statement order, and why it is still priced

Some gapped pools are not a cross-function order problem at all — every literal belongs to the
same function in the same sequence, and only the order *within* one function differs. Those look
like a free win (reorder statements, keep the code) and they are not. `engine/69`
(`CameraModeTalk_update`, gap 64, the whole `.sdata2`) is the clean specimen. Retail mints
`6.0, 0.2, 50.0, 25.0, 0.0625` but *loads* them `6.0, 0.2, 25.0, 50.0, 0.0625`: the `25.0f` is a
local assigned before the clamp (hoisted load), the `50.0f` is a literal inside the expression
(load at use). To mint `50.0f` first, the source needs a **live** `50.0f` ahead of the
`followTermB = 25.0f;` statement — and every live form moves its load:

| probe | `.sdata2` 0x2c/0x30 | `CameraModeTalk_update` |
| --- | --- | --- |
| tip | wrong order | **100.0** |
| `followDist = 50.0f;` temp, used in the expression | fixed | 98.773 |
| drop the `followTermB` temp, write `(50.0f + 25.0f * heightT)` | fixed | 99.033 |
| `followDist = 50.0f;` as a **dead** store, expression unchanged | unchanged | 99.907 |

The last row is the general fact worth keeping: **a dead store is eliminated before literal
minting and mints nothing**, so it is useless as well as a phantom minter. Every intra-function
row therefore has the same shape — the mint order is only reachable through a live use, and a
live use is exactly what moves the load. `engine/69` is PRICED 64 B; `578_DBstealerwo`,
`main/trig`, `609_DR_LaserCan`, `engine/19`, `429_SH_thorntai`, `202/mikaladon` are the same
sub-class.

Classifier for a new row (`.sdata2` all-or-nothing, so a partial pool fix scores zero): map each
pool slot to the function of its first reference on both sides. Same function sequence with
different first-reference offsets = intra-function (8b). Different function sequence = the
cross-function mint order of §8.

## 9. The opaque-extern crutch: the oracle, the tree-wide census, and the plain-literal price

§8 named one instance of this shape — `engine/68`'s `gCameraModeViewfinderStickScale`, an
`extern const f32` that no translation unit defines, whose only working role is to be opaque
enough to stop `-opt propagation`. `90b1a0f251` then closed five instances of it by defining the
constant in the unit that reads it. This section is the tree-wide sweep behind that class: the
oracle that tells a crutch from an honest cross-TU reference, what is left after `90b1a0f251`,
and a measured price for the plain-literal route so it is not tried again unpriced.

### The oracle: `nm` retail's own split object

The tree carries ~3040 `extern` object declarations. 371 name a symbol **no object in the tree
defines**; 211 of those are scalars in game code (`src/main`, `src/track`, `src/dlls`) that
`symbols.txt` places in `.sdata2`. Neither "nothing defines it" nor "it is in `symbols.txt`"
separates a crutch from an honest reference: the honest form is also undefined by us, and
`symbols.txt` carries no linkage information at all.

What separates them is the **retail split object for the same unit**:

- retail's `.o` also lists the symbol `U` → retail's own TU referenced it across a TU boundary,
  and our `extern` is a faithful reconstruction. `engine/75`'s eleven `gCamClimb*` are this
  case: retail's `75.o` UNDs every one of them.
- retail's `.o` **defines** it (`R`/`D` at an offset) → retail's TU minted that word itself and
  we did not. The `extern` is standing in for a mint. That is the crutch, and it is also a
  latent link failure, because only the carve is supplying the symbol.

The sweep is cheap and exhaustive, and its result is mostly a **retirement**: of the 211 rows,
**148 are honest** and should never be flagged by a later census. **71 were crutches** at
`8c2eb8998a`; `90b1a0f251` closed five (`render`, `589_BossDrakor`, and one slot each in
`engine/68`, `vecmath`, `engine/5`), leaving **66 in 10 units** at that tip:

| unit | crutches | pool words missing / extra | data at `90b1a0f251` |
| --- | --- | --- | --- |
| `dlls/engine/0/0` | 20 | 18 / 1 | 8972/9952 |
| `main/object` | 16 | 18 / 0 | 520/608 |
| `main/newshadows` | 12 | 13 / 0 | 16388/16668 |
| `main/model` | 7 | 9 / 1 | 496/612 |
| `dlls/objects/195_Player/player` | 3 | 0 / 0 | 10168/10168 |
| `main/objhits` | 2 | 2 / 0 | 8352/8440 |
| `main/vecmath` | 2 | 3 / 0 | 0/84 |
| `dlls/engine/5/5` | 2 | 1 / 0 | 600/776 |
| `dlls/engine/68/68` | 1 | 1 / 1 | 64/192 |
| `dlls/objects/704/704` | 1 | 0 / 1 | 720/884 |

The census reproduces the ELF-level pool shortfall from the other side — `engine/0`,
`main/object`, `main/newshadows` and `main/model` are the top rows of both — which is the useful
consequence: **most of the missing-word debt in those four units is this shape, not a naming gap.**

### The plain-literal route is priced everywhere (negative control)

Replacing every crutch read in a unit with the plain literal carried in retail's own pool word,
rebuilding only that object, all other gates unchanged. Measured at `8c2eb8998a`, before
`90b1a0f251` reworked five of these slots; the rows are kept because they are the control the
`const`-definition route should be judged against, and because nine of the ten recover nothing.

| unit | tree fuzzy | tree matched_data | worst function |
| --- | --- | --- | --- |
| `engine/0/0` | 99.81176 -> 99.79668 | 1201521 -> **1196585 (-4936)** | `pauseMenuDrawTaskHintPanel` 100.0 -> 71.028 |
| `main/newshadows` | -> 99.80027 | +0 | `evalNoisePlacements` 100.0 -> 87.057 |
| `main/model` | -> 99.80949 | +0 | `Model_GetVertexPosition` 100.0 -> 73.033 |
| `main/vecmath` | -> 99.81110 | +0 | `mtx44_multSafe` 100.0 -> 98.640 |
| `main/objhits` | -> 99.80982 | +0 | `ObjHits_CheckSkeletonPair` 99.247 -> 96.792 |
| `main/object` | -> 99.81141 | +0 | `modelInitBones` 100.0 -> 98.267 |
| `dlls/objects/704/704` | -> 99.81149 | +0 | `titleScreenDrawMenuFrame` 99.776 -> 99.488 |
| `589_BossDrakor` | -> 99.81158 | +0 | `bossdrakor_update` 99.854 -> 99.626 |
| `engine/68` | -> 99.80965 | 1201521 -> 1201649 (+128) | `firstPersonDoControls` 100.0 -> 94.512 |
| `engine/5/5` | -> 99.81089 | 1201521 -> 1201697 (+176) | `renderSunAndMoon` 99.476 -> 98.214 |

`engine/68` reproduces §8's figures exactly on an independent harness, which is the control for
the rest of the table. Three facts fall out:

1. **The all-or-nothing law holds in both directions.** Nine rows recover **zero** data even
   where the literal lands in the right slot, because one displaced word voids the section.
   `engine/0` is the reverse case and the one to be careful with: inserting 20 correct words
   into a section that was already scoring **destroyed 4936 bytes**. A partial pool fix is not
   merely worth 0, it can be worth far less than 0.
2. **Where the pool already matches byte-for-byte, the crutch is buying code and nothing else.**
   `player`, `704` and (before `90b1a0f251`) `BossDrakor` already emit the very word the extern
   points at; the extern's only effect is to force a separate opaque load instead of letting
   MWCC fold or CSE the literal. Removing it costs 0.06-0.29 on a function for no data at all,
   so those three rows are ban-reduction with no compensating recovery.
3. The plain literal is therefore the wrong instrument. `90b1a0f251`'s rule is the operative one:
   a named `.sdata2` symbol is materialised after the other operand and is not CSE'd, an interned
   literal is materialised before it and is, so the choice is a `.text` decision first.

### `engine/5`'s last word: reachable, and priced (measured at `90b1a0f251`)

`engine/5` is one slot from a byte-identical pool: retail's `0.55f`
(`gSkySunMoonRiseScale`) at `.sdata2+0x5c`, a front-end literal of `renderSunAndMoon` minted
between `28800.0f` (0x58) and `2.0f` (0x60). It cannot be reached by a definition, because a
file-scope object mints at its **declaration point** and 0x5c sits in the middle of the
function's own literal run — no declaration position exists that lands there. Writing the plain
literal does not reach it either: `scale` is single-use and `-opt propagation` sinks the whole
assignment past the `2.0f` and `400.0f` statements to its consumer, so `0.55f` mints two slots
late. Splitting the statement blocks the sink and is the only form that reaches the slot:

```c
scale = 0.55f * riseT;
scale = 1.0f - scale;
```

`.sdata2` then goes byte-identical, **600/776 -> 776/776**, tree 99.81176 -> 99.81078,
matched_data **1203257 -> 1203433 (+176)**, `renderSunAndMoon` 99.476 -> 98.018. The `-(x - 1.0f)`
spelling of the same split is worse (97.608), and a *named* second temp (`riseScale`) does not
work at all — it changes the liveness, the sink returns and the pool goes back to three wrong
words. Left unlanded: it trades 0.00098 fuzzy for 176 bytes and one opaque extern, and that is
the owner's trade, not a lane's.

### Do not re-survey

The 148 honest rows are settled by the oracle; a later census should not re-flag them.
`main/render` is the one structurally unreachable row even after `90b1a0f251`: retail's
`.sdata2` there is 64 bytes of which 60 are `pad_11_803DE508_sdata2`, another TU's data, so the
section stays 15 words short (9040/9104) no matter how its own constant is spelled.


## 9b. Which crutch slots a declaration point can actually reach

Section 9 sized the crutch class and priced the plain-literal route. It left one question open,
and the whole remaining decision rests on it: for a given crutch, is retail's pool slot
**reachable from a declaration point** - so that the const route of `90b1a0f251` could in
principle put a word there - or is it a front-end literal minted in the middle of one function's
own run, where no declaration can land? Section 9 answered that for a single instance
(`engine/5`'s `0.55f` at 0x5c, measured mid-run) by trying it. This section answers it for all
of them, and it starts by correcting the model everyone has been reasoning from.

### The declaration point is NOT the head of the pool

The natural reading of the `objhits` experiment - two file-scope consts declared together at the
top emitted at 0x000 and 0x004 - is that declarations land at the head of `.sdata2` and the
minted literals follow. **That is wrong, and it is wrong in the direction that matters.** Three
`const f32` objects declared at three different points of `dlls/engine/68/68.c` - before the
first function, after `firstPersonPlaceCamera`, and after `firstPersonEnter` - emit at

    0x04    declared before the first function
    0x0c    declared after the first function
    0x60    declared after the third function

interleaved with the literal groups of the functions they sit between. The tree already contains
the same evidence from a landed commit: `90b1a0f251`'s `gVecMathAngleScaleInv`, declared at
`vecmath.c:218`, sits at `.sdata2` **0x30**, not at 0x00. A file-scope const emits **at its
declaration position in the translation unit**. The `objhits` pair landed at 0x000/0x004 only
because both were declared in the same place.

So the reachable set is not "the first word of the pool" (which would be three slots out of
fifty-one). It is **every slot that falls on a boundary between two functions' mint groups**.

### Reading the boundaries off retail's own object

Retail's split object gives the groups directly. Take its `.sdata2` symbols in offset order and
its `.text` relocations; each pool slot is referenced by a set of functions, and walking the
slots in order while holding a current owner - keep it if it still references this slot,
otherwise advance to the next function that does - segments the pool into contiguous mint groups
in `.text` order. A crutch is at a **declaration point** when the last non-crutch slot before it
and the first non-crutch slot after it are minted by *different* functions (or when it is at the
head or tail of the section); it is **mid-run** when both neighbours are minted by the same one.

The segmentation is calibrated against the three instances other lanes have already measured,
and it reproduces all three:

| slot | measured elsewhere | this model |
| --- | --- | --- |
| `engine/5` 0x5c `gSkySunMoonRiseScale` | section 9: front-end literal mid-`renderSunAndMoon` | mid-run |
| `engine/68` 0x44 `gCameraModeViewfinderStickScale` | section 8: no declaration point reaches it | mid-run |
| `704` 0x18 `lbl_803E2300` | section 9: plain literal recovers 0 | mid-run |

### The table

51 crutches in 9 units at `10b2cb641b` (`main/object`'s 16 were closed by `ca33bc08`).
**26 sit at a declaration point; 25 are mid-run.**

**`dlls/engine/0/0`** - 20 crutches, 9 at a declaration point, 11 mid-run

| slot | symbol | verdict | run it sits in |
| --- | --- | --- | --- |
| 0x0094 | `gGameUiAngleDivisor` | mid-run | inside `pauseMenuSetHoloTransform` |
| 0x00c0 | `hudElementOpacity` | mid-run | inside `drawViewFinderHud` |
| 0x00c4 | `lbl_803E1EC4` | mid-run | inside `drawViewFinderHud` |
| 0x00c8 | `gGameUiPi` | mid-run | inside `drawViewFinderHud` |
| 0x0130 | `lbl_803E1F30` | mid-run | inside `drawViewFinderHud` |
| 0x0134 | `lbl_803E1F34` | mid-run | inside `drawViewFinderHud` |
| 0x0170 | `gViewFinderBamToDeg` | mid-run | inside `drawViewFinderHud` |
| 0x01bc | `gHudElemOpacityFloor` | **declaration point** | `hudDrawCounter` / `pauseMenuDrawStatus` |
| 0x0210 | `lbl_803E2010` | mid-run | inside `hudDrawButtons` |
| 0x024c | `lbl_803E204C` | mid-run | inside `headDisplayDraw` |
| 0x029c | `lbl_803E209C` | mid-run | inside `pauseMenuDraw` |
| 0x02b8 | `lbl_803E20B8` | **declaration point** | `pauseMenuDrawStatusPage` / `pauseMenuDrawSideRails` |
| 0x0308 | `gPauseMenuGridCursorScale` | mid-run | inside `pauseMenuDrawGrid` |
| 0x0328 | `lbl_803E2128` | **declaration point** | `pauseMenuDrawGridCell` / `timeListDraw` |
| 0x0378 | `gPauseMenuPodiumRollAmplitude` | **declaration point** | `pauseMenuRunSubmenu` / `pauseMenuAnimateCarousel` |
| 0x037c | `gPauseMenuPodiumBaseY` | **declaration point** | `pauseMenuRunSubmenu` / `pauseMenuAnimateCarousel` |
| 0x0380 | `gPauseMenuPodiumBobAmplitude` | **declaration point** | `pauseMenuRunSubmenu` / `pauseMenuAnimateCarousel` |
| 0x038c | `gPauseMenuCommunicatorMaxScale` | **declaration point** | `pauseMenuAnimateCarousel` / `mapScreenDrawHud` |
| 0x0390 | `gPauseMenuRingScale` | **declaration point** | `pauseMenuAnimateCarousel` / `mapScreenDrawHud` |
| 0x0394 | `gPauseMenuRingUnselectedScale` | **declaration point** | `pauseMenuAnimateCarousel` / `mapScreenDrawHud` |

**`dlls/engine/5/5`** - 2 crutches, 1 at a declaration point, 1 mid-run

| slot | symbol | verdict | run it sits in |
| --- | --- | --- | --- |
| 0x0000 | `lbl_803DF058` | **declaration point** | `head of pool` / `skySetLightIndex` |
| 0x005c | `gSkySunMoonRiseScale` | mid-run | inside `renderSunAndMoon` |

**`dlls/engine/68/68`** - 1 crutches, 0 at a declaration point, 1 mid-run

| slot | symbol | verdict | run it sits in |
| --- | --- | --- | --- |
| 0x0044 | `gCameraModeViewfinderStickScale` | mid-run | inside `firstPersonDoControls` |

**`dlls/objects/195_Player/player`** - 3 crutches, 1 at a declaration point, 2 mid-run

| slot | symbol | verdict | run it sits in |
| --- | --- | --- | --- |
| 0x003c | `lbl_803E7EA4` | mid-run | inside `playerUpdateTail` |
| 0x0078 | `lbl_803E7EE0` | **declaration point** | `playerCastSpell` / `playerGetAimAngles` |
| 0x00ac | `lbl_803E7F14` | mid-run | inside `playerState3D` |

**`dlls/objects/704/704`** - 1 crutches, 0 at a declaration point, 1 mid-run

| slot | symbol | verdict | run it sits in |
| --- | --- | --- | --- |
| 0x0018 | `lbl_803E2300` | mid-run | inside `titleScreenDrawMenuFrame` |

**`main/model`** - 7 crutches, 7 at a declaration point, 0 mid-run

| slot | symbol | verdict | run it sits in |
| --- | --- | --- | --- |
| 0x0024 | `gModelDotClampMax` | **declaration point** | `modelAnimResetState` / `modelChainUpdateNodesPassive` |
| 0x0028 | `gModelDotClampMin` | **declaration point** | `modelAnimResetState` / `modelChainUpdateNodesPassive` |
| 0x0040 | `gModelPhaseWrapPeriod` | **declaration point** | `modelChainApplyDampingAndJitter` / `ObjModel_ApplyBlendChannels` |
| 0x0044 | `gModelDefaultOriginX` | **declaration point** | `modelChainApplyDampingAndJitter` / `ObjModel_ApplyBlendChannels` |
| 0x0048 | `gModelDefaultOriginY` | **declaration point** | `modelChainApplyDampingAndJitter` / `ObjModel_ApplyBlendChannels` |
| 0x004c | `gModelDefaultOriginZ` | **declaration point** | `modelChainApplyDampingAndJitter` / `ObjModel_ApplyBlendChannels` |
| 0x0050 | `gModelVertexScale` | **declaration point** | `modelChainApplyDampingAndJitter` / `ObjModel_ApplyBlendChannels` |

**`main/newshadows`** - 14 crutches, 5 at a declaration point, 9 mid-run

| slot | symbol | verdict | run it sits in |
| --- | --- | --- | --- |
| 0x0028 | `gNewShadowFovY` | **declaration point** | `renderObjectShadowTexture` / `renderShadows` |
| 0x0030 | `lbl_803DED38` | mid-run | inside `renderShadows` |
| 0x0038 | `lbl_803DED40` | mid-run | inside `renderShadows` |
| 0x0070 | `gNewShadowAspectWide` | mid-run | inside `renderShadows` |
| 0x0074 | `gNewShadowAspectNarrow` | mid-run | inside `renderShadows` |
| 0x00b8 | `lbl_803DEDC0` | mid-run | inside `createNewShadowDistortionTexture` |
| 0x00c8 | `lbl_803DEDD0` | **declaration point** | `createNewShadowDistortionTexture` / `evalNoisePlacements` |
| 0x00d8 | `lbl_803DEDE0` | mid-run | inside `newShadowsInitProceduralTextures` |
| 0x00e8 | `lbl_803DEDF0` | mid-run | inside `allocLotsOfTextures` |
| 0x00ec | `lbl_803DEDF4` | mid-run | inside `allocLotsOfTextures` |
| 0x00f4 | `lbl_803DEDFC` | mid-run | inside `allocLotsOfTextures` |
| 0x010c | `lbl_803DEE14` | **declaration point** | `allocLotsOfTextures` / `end of pool` |
| 0x0110 | `lbl_803DEE18` | **declaration point** | `allocLotsOfTextures` / `end of pool` |
| 0x0114 | `lbl_803DEE1C` | **declaration point** | `allocLotsOfTextures` / `end of pool` |

**`main/objhits`** - 2 crutches, 2 at a declaration point, 0 mid-run

| slot | symbol | verdict | run it sits in |
| --- | --- | --- | --- |
| 0x0000 | `gObjHitsScalarZero` | **declaration point** | `head of pool` / `ObjHits_InitWorkBuffers` |
| 0x0008 | `gObjHitsScalarOne` | **declaration point** | `ObjHits_InitWorkBuffers` / `ObjHits_CheckTrackContact` |

**`main/vecmath`** - 1 crutches, 1 at a declaration point, 0 mid-run

| slot | symbol | verdict | run it sits in |
| --- | --- | --- | --- |
| 0x0000 | `lbl_803DE7C0` | **declaration point** | `head of pool` / `interpolate` |

### What the table does and does not license

A **declaration point** verdict is necessary, not sufficient. It says the slot's *position* is
reachable; it says nothing about the *read*. A plain `const f32` read folds at `-O4`, and the
folded value mints a duplicate further down the pool - that is what the `objhits` probe showed,
and it is why the only spelling that has actually reached one of these slots is the
single-element const array. That pin is adjudicated per instance by the const lane
(section 8's retraction), and nothing here changes who decides it.

A **mid-run** verdict is a real closure. It says the word was minted by a literal written inside
that function's body, so the fix is in the function, not in any declaration - and 25 of the 51
crutches are in that class, including every crutch in `engine/68` and `704` and eleven of
`engine/0`'s twenty. Those rows should stop being counted as candidates for the const route.

Two further consequences worth recording. `main/model`'s seven crutches are **all** at
declaration points, in two adjacent runs (0x24-0x28 and 0x40-0x50) - the cleanest remaining unit
of the class,
and section 9 measured its plain-literal route at 0 recovered data, so nothing about it has been
tried at the right instrument yet. And `dlls/engine/0`'s nine reachable slots fall into four
adjacent runs (0x1bc, 0x2b8, 0x328, 0x378-0x380, 0x38c-0x394), which matters because section 9's
decisive negative there - twenty correct words inserted **lost 4936 bytes** - was measured on the
all-or-nothing literal route across the whole section, not on those runs.

## 10. The pool is a fossil of the source text, not of the code (measured 2026-08-03)

Sections 8, 8b and 9b all try to predict a `.sdata2` slot order from something the compiler
emits — the order functions appear in `.text`, the order a function's own run references its
words, the position of a declaration. Section 8's author already retracted the strongest claim,
and a later lane recorded that the model has "a hole" it could not name: `main/acosf` and
`track/intersect_render` sit at 75.410 and 91.525 with retail's mint groups in an order that is
a *permutation* of `.text` order, which no source edit appears able to produce.

There is a one-command test that closes it, and the answer is that the premise is wrong.

### The value-sequence oracle

For each function, walk its `.text` in address order and write down the **value** of every
`.sdata2` word it references, in that order. Do it for our object and for the retail carve, and
compare the two sequences. If they are equal, the two objects' code asks for exactly the same
constants in exactly the same order, and only the slot each constant lives in differs.

`tools/pool_value_sequence.py <src> [section]` does this, and `--all` runs it over every
sub-100 data section in the report. At `e173a2c951`:

| result | sections |
| --- | --- |
| every function's value sequence **identical** to retail's | **22** |
| some function's sequence differs | 11 |

`main/acosf` (8 functions, sequences of 6 to 36 words), `track/intersect_render` (31 functions),
`main/object`, `main/rcp_dolphin`, `main/shader_dolphin`, `engine/19`, `SmallBasket`,
`AppleOnTree`, `objlib` and thirteen more are all in the first row. **Their `.text` is already
saying, word for word, what retail's says.** The pool is nevertheless laid out differently.

So the slot order is not a function of the emitted code at all. It is fixed by the front end,
from the order the literals appear in the **source text**, before the code generator runs — and
the code generator is free to schedule the loads into any order it likes afterwards. That is why
a function can sit at 100.0 with a rotated pool, and why no amount of register, schedule or
statement-level work moves the section: there is nothing in the generated code left to fix.

The corollary is the useful part. **Run the oracle before spending any effort on a sub-100 data
section.** All-identical means the only remaining lever is the source *text* — which expression
was written where — and that is a decompilation question, not a codegen one. A differing
sequence means there is still a real value or a real reference to recover first, and that is
where the work belongs.

### Two sub-rules, both measured

Within one statement the intern order is neither left-to-right nor evaluation order; it is
**increasing expression-tree depth**. `704`'s

```c
gTitleScreenPulseAlpha = 127.0f * mathCosf(3.142f * (2.0f * t) / 100.0f) + 128.0f;
```

mints `128.0f, 127.0f, 3.142f, 2.0f` — the depths 1, 2, 5, 6 of that tree, with `100.0f` skipped
because an earlier statement already interned it. `trig`'s `fsin16Approx` mints
`0.99999f, -2.8707542e-10f, 1.3332733e-20f` from `x2 * (C2 * x2 + C1) + C0`, again shallowest
first. Both reproduce exactly.

An 8-byte literal is **deferred to the next 8-aligned offset, and the 4-byte hole it leaves is
back-filled by the next 4-byte literal interned.** `704`'s `titleScreenShowCopyright` references
`1.0f, 0.9999f, 80.0f, D(bias), 255.0f` in that order and lands them at
`0x00, 0x04, 0x08, 0x0c=255.0f, 0x10=D` — the double stepped over `0x0c` and `255.0f` took it.
This is the mechanism behind the "the displaced item is always a double or the word beside one"
observation, and behind an off-by-one-word section size: a pool whose doubles are interned one
slot earlier or later needs a pad where retail needed none.

### What this costs the terminal rows

`main/acosf` 248 B and `track/intersect_render` 236 B are terminal for this reason and no other:
their code is already correct, so the residual is purely which statement of which function was
written where in retail's `.c`. Nothing in this repository's normal toolkit reaches it. They
should be scored as **closed on the code axis and open only on the text axis**, and a lane that
wants them must go after the source text — not the registers.

## 10b. Pricing the opaque-extern crutch PER SYMBOL, not per unit

Section 9's control table prices the plain-literal route one unit at a time: replace every
crutch read in a unit, rebuild, measure. Nine of its ten rows recover nothing, and the section
concludes that the plain literal is the wrong instrument everywhere. `ca33bc08` had already
shown one unit escaping that verdict (`main/object`, 16 crutches, free once `modelInitBones`'s
`zero` temp was hoisted above the radius compare).

Priced **per symbol** the picture changes again: a unit's price is almost never spread across
its crutches, it is concentrated in two or three of them. Sweeping all 51 crutches individually
at `f64f18ca9f`, one symbol at a time, rebuilding only that object and regenerating the whole
report:

| unit | crutches | free | priced | the price |
| --- | --- | --- | --- | --- |
| `main/model` | 7 | **6** | 1 | `gModelVertexScale` -> `Model_GetVertexPosition` 100.0 -> 73.033 |
| `main/newshadows` | 14 | **3** | 11 | every one of the 11 moves `allocLotsOfTextures` |
| `dlls/engine/0/0` | 20 | **6** | 14 | `lbl_803E209C` alone costs **4936** bytes of matched_data |
| `dlls/objects/195_Player/player` | 3 | 0 | 3 | `lbl_803E7EE0`/`lbl_803E7F14` cost 8232 B of `.data` |
| `main/objhits` | 2 | 0 | 2 | ~100 read sites, `ObjHits_CheckSkeletonPair` |
| `main/vecmath` | 1 | 0 | 1 | `mtx44_multSafe` 100.0 -> 98.639 |
| `dlls/engine/5/5` | 2 | 0 | 2 | `renderSunAndMoon` |
| `dlls/engine/68/68` | 1 | 0 | 1 | `firstPersonDoControls` 100.0 -> 94.512 (+128 data) |
| `dlls/objects/704/704` | 1 | 0 | 1 | `titleScreenDrawMenuFrame` 99.776 -> 99.488 |

`b93a5f226d` landed the 15 free ones: tree fuzzy 99.811850 -> 99.811966, matched_data held,
0 REGRESSED, 2 IMPROVED, and the missing-word count in those three units halved (engine/0
18 -> 8, newshadows 13 -> 9, model 9 -> 3). **Section 9's per-unit table is an upper bound on
the price, not the price.** Any later census of this class should sweep one symbol at a time.

### Two distinct prices, and only one of them is a colouring

`ca33bc08`'s lesson - that the regression is the register colouring and an ordinary source move
fixes it - does not generalise. The 36 priced rows split cleanly:

- **Address-priced.** A self-owned `lbl_XXXXXXXX` extern resolves to retail's own address, so
  every reference through it is a *guaranteed* hit. Replace it with a literal and the reference
  moves to wherever our own pool happens to put that word. `704` is the pure case: with the
  crutch gone the instruction streams are identical (T=836, C=836, every region a reloc name),
  and the whole 0.288 is the address. **These rows are gated behind fixing the pool order
  first**, after which they become free by construction.
- **Schedule-priced.** A load of a named global is a memory reference the instruction scheduler
  will not move across; an interned `@NNN` literal is not, and MWCC hoists it. `engine/68`'s
  `firstPersonDoControls` is the clean instance: `spinI = (int)(15360.0f * ((f32)stickY /
  120.0f))` and the `camera->anim.rotX` statement after it swap wholesale, T=306 C=306.
  Swapping the two statements in the source, and splitting the division into its own temp, both
  reproduce **the identical 94.512** - the scheduler is deciding, not the text. No source move
  reaches these; they are as priced as section 9 says.

## 11. The front-end storage-order laws, and which text-axis rows they reach (measured 2026-08-03)

Sections 8, 8b and 10 model the `.sdata2` slot order and conclude that it is a fossil of the
source text. This section is the constructive half: what every file-scope construct actually
emits and where, which sub-100 rows that reaches, and a second oracle - `.bss` - that asks the
same question for free.

### What each construct emits, and where

Measured with single-TU probes under both live flag sets (`-O4,p -opt nopeephole,noschedule
-inline noauto` for `src/main`, `-O4,p -opt nopeephole,noschedule,nopropagation -inline auto`
for the DLLs); both sets agree.

| construct | what lands in `.sdata2` | what the read compiles to |
| --- | --- | --- |
| plain literal in an expression | a word where the expression is lowered | that word |
| `static const f32 X = V;` | **nothing** - folded, then dead-stripped | a duplicate literal at first use |
| `const f32 X = V;` (external linkage) | a word **at the declaration point** | a duplicate literal at first use; the object itself is never referenced |
| `static const` aggregate (struct or `[2]`), size <= 8 | a word at the declaration point | a real SDA21 load - no fold, no duplicate |
| any aggregate larger than 8 bytes | `.rodata`, never `.sdata2` | - |
| `static __inline` fn defined above the users | its literals intern **at the use site** | - |
| plain `static` fn defined above the users | its literals intern at the **definition**, but MWCC also emits the body into `.text` | - |

Two consequences. First, the only construct that places a word at a chosen position while
emitting zero instructions is the external-linkage scalar `const`: it is a pure **insertion**,
able to add a word between any two mint points and unable to move an existing one. Second, the
`static` function is not a usable phantom minter - the body is emitted, and
`banned_shapes_check.py` scans uncalled statics anyway.

Because nothing references an inserted const, `mwld` dead-strips it and the pool shortens again,
shifting every later SDA2 reference and breaking the DOL. The symbol must be listed in
`config/GSAE01/config.yml` `force_active`; the entries already there
(`lbl_803E06C4`, `gGcRobotPatrolZero`, `gDIMSnowHorn1ZeroOffset`, `lbl_803E3E44`, ...) are the
same class, and the tree already carries six of these consts in game code
(`gMikaBombZero`, `gDll76Zero`, `gDll77Zero`, ...).

### The insertion classifier, run over the whole frontier

`tools/pool_value_sequence.py --all` says which sections are open only on the text axis. A second
pass says which of those the const lever can actually close: diff the two slot sequences (size,
bytes) with an LCS and ask whether every opcode is `equal` or `insert`, and whether each inserted
word is referenced on retail's side. Over all 33 sub-100 data sections at `590dce7361`:

| row | verdict |
| --- | --- |
| `main/objlib` | one **unreferenced** zero word at slot 1. CLOSED - `.sdata2` 91.667 -> 100.0, +48 `matched_data` |
| `300_Transporter` | not a pool row at all - see below. RETIRE |
| `engine/5`, `engine/68` | INSERT-ONLY, but the inserted word is **referenced**, so it needs a real minter; these are exactly the priced crutch rows of section 9 |
| `main/render` | the insertion is the 60-byte `pad_11_803DE508_sdata2` blob - a splits-ownership question, not a text-axis one |
| the other 28 | need words **moved**; no file-scope construct moves a literal |

### `300_Transporter` is a carve-attribution artifact, not a gap

Both `.sdata2` sections carry `2**3` alignment (each holds a bias double). Our object's section is
0x4c bytes and `302`'s starts 8-aligned at `0x803E3EE8`, so `mwld` inserts four bytes of padding
that `splits.txt` attributes to Transporter's range; objdiff compares 76 bytes against 80 forever.
The DOL is byte-identical either way. The row should leave the frontier rather than be "fixed"
with a fabricated tail object.

### `.bss` order is a second, free oracle for the same question

Declaration order is **completely inert** - a 36-cell declaration x use matrix (all six
declaration permutations of three `.bss` arrays against all six use permutations) gives results
that depend only on the use order. The law is **first-use order**, where "use" is the front end's
and not the code generator's:

- uses in separate statements, or in separate functions: layout == first-use order exactly;
- three uses inside one expression `A[i] + B[i] + C[i]`: layout `C A B`, the shallowest
  sub-expression first. That is section 10's increasing-tree-depth rule, now confirmed on a
  second kind of storage.

So `.bss` asks the source-text question for the price of one `objdump -t`, and on a mover row it
corroborates the pool. `main/objlib` is the specimen: retail's `.bss` is `gObjectTypeList,
gObjectTypeIndices, gObjContactCallbacks`, ours is `gObjectTypeIndices, gObjectTypeList,
gObjContactCallbacks`, and the two spellings that do fix the order (`entry = gObjectTypeList;`
hoisted, and `entry = gObjectTypeList + (index = gObjectTypeIndices.offsets[group]);`) both lift
the list base's `lis`/`addi` above the `limit` load and break `objIsObjectType`. `.text`,
`.bss` and `.sdata2` therefore disagree about the source order of the same file - which is the
sharpest statement yet of what section 10 found, and it is why `main/objlib` stays `NonMatching`
even with a byte-identical `.sdata2`: flipping it links our `.bss` order and moves 33 DOL words.

### Phantom minters: the negative results

None of these interns anything (single-TU probes): a dead local initialiser `f32 t = 100.0f;`,
a dead store `t = x * 200.0f; t = 0.0f;`, an unused `(f32)n` conversion (no bias double), a
function-local `const f32 t = 300.0f;`, and `x * 500.0f * 0.0f` (folded). The one shape that does
mint a word it never loads is a dead **comparison** - `(x > 400.0f) ? x : x` emits `400.0f` - so
the front end interns when it lowers a compare, not when it folds arithmetic. It is not plausible
C and is recorded only to close the search.

## 12. The mover: which construct can put a pool word ahead of its first live loader (measured 2026-08-02)

Sections 10 and 11 established that `.sdata2` order is the source text's order, and that no
file-scope insertion can reorder two words that a function body already minted. This section
closes the remaining question: when retail's pool holds a word **ahead of the first function that
loads it**, what did the source do? Twenty-eight sub-100 data sections need exactly that.

### The survey

Of the 32 sub-100 data sections at `98445c52ec`, the value multisets already agree on 12
(`PURE-ORDER`); of the rest, most of the "missing" words are `00000000` at an offset that is the
alignment hole in front of an eight-byte double, so the bytes are already identical and only the
symbol granularity differs. The residue is genuinely an ordering problem, and in every case the
words that have to move are ones whose only loads are in a *later* function.

### The construct table, re-measured on `main/rcp_dolphin`

`Rcp_InitDistortionEffects` is the last function in the unit and its three constants are the
first three pool words; the section scores 14.63%. Four spellings, each one build:

| spelling | where the word lands | how it is loaded |
| --- | --- | --- |
| plain literal | at the use | direct SDA21 |
| `static const f32 x = V;` | at the use (folded, re-minted) | direct SDA21 |
| `const f32 x = V;` (external) | at the declaration **and** at the use | the *duplicate* is loaded |
| `static const` aggregate <= 8 B | at the declaration | direct SDA21, no duplicate |

Only the last one moves a word. A twelve-byte aggregate goes to `.rodata` instead, and a member
at a non-zero offset costs a base-register materialisation (`addi rN,r2,sym` + `lfs f,4(rN)`)
where retail has one `lfs`, so each word has to be its own symbol at offset zero. With three
such declarations placed before the first function, `main/rcp_dolphin`'s `.sdata2` goes
14.63% -> 100% and **every function stays byte-identical**; the same lever takes
`279_AppleOnTree` 90.62% -> 100% (three words: `1.0f` and the two halves of the fall-scale
blend) and `701` 95.65% -> 100% (one word: the zero `AndrossHand_update` and `AndrossHand_init`
share, declared between `AndrossHand_free` and `AndrossHand_render`). Measured together:
`matched_data` 1203317 -> 1203617, `complete_units` 906 -> 909, `fuzzy_match_percent` unchanged,
`main.dol` OK, and md5 of every `.o` identical outside the three units.

**That spelling is `const T name[1] = {V}`, which `tools/banned_shapes_check.py` gates as
`SINGLE_ELEM_CONST_ARRAY`.** So the lever exists, it is exact, and it is the banned one. Landing
these three rows is a decision for whoever owns that baseline; the measurement is recorded here so
the decision can be made on numbers.

### Why there is no third option

The two candidate origins for a word ahead of its first live loader are a file-scope constant and
a dead static that `mwld` stripped (`UNCALLED_STATIC_FN`, and see that check's own rationale).
Both are gated. Nothing else reaches: a declared constant is **never** interned against the
literal pool - declaring `static const f32 sCrTestOne[1] = {1.0f}` in `362_CRrockfall` while a
`1.0f` literal survives elsewhere in the unit emits **two** words, at `0x0008` and `0x000c`.

### What statement order inside a function can and cannot do

Section 11's text-axis model predicts that a literal moves with the statement that uses it. It
does, but only when the *use* moves - the front end interns at the use, not at an assignment the
optimiser will propagate away:

- `dlls/engine/24` wants `boneParticleEffect_update`'s `0.0f` first. Hoisting `zero = 0.0f;` from
  the loop preamble to the top of the function changes **nothing** - not one pool byte, not one
  `.text` byte - because the assignment is propagated and the load is re-materialised at the
  three `vtx.x = zero;` uses.
- `main/vecmath` wants `interpolate`'s `0.0f` ahead of its `1.0f`, with retail's code an exact
  match for the `if (t <= 1.0f) { ... } return 0.0f;` shape we already have. `f32 result = 0.0f;`
  plus a single exit reaches the pool order but costs the function 100 -> 87.08; `f32 result =
  0.0f;` keeping the early return is inert, confirming section 11's dead-initialiser result.

So an intra-function mover needs a genuinely different *use* order, which is a semantic
rewrite, not a reordering - and on these rows retail's own code shape rules it out.

### Rows that the lever cannot reach at all

`328_CFGuardian` and `362_CRrockfall` each need a compiler-generated **bias double** moved ahead
of an earlier function's literals (`CFGuardian`: `4330000080000000` before `cfguardian_flyAlongPath`'s
`200.0f`, whose code is byte-identical to retail; `CRrockfall`: `4330000000000000` ahead of
`crrockfall_findFloorY`, the unit's *first* function). A bias is minted by the code generator, so
no declaration can place it; these are dead-static rows or nothing. `main/shader_dolphin` needs
`0.0f` hoisted, and the file has 181 `0.0f` literals - every one would have to be rewritten for
the pool to hold a single word, which is not plausible source at any price.

### 12b. The spelling census: the ban is on the shape, not on the bracket (measured 2026-08-03)

Section 12 left one question open: its mover is `const T name[1] = {V}`, which
`tools/banned_shapes_check.py` gates as `SINGLE_ELEM_CONST_ARRAY`, so it asked whether some
other aggregate spelling reaches the same layout without tripping the checker. Seven spellings
were measured, one build each, on `701` -- the smallest specimen, one word (`0.0f`) wanted at
`.sdata2+0x0c` ahead of `AndrossHand_render`'s `1.0f`:

| spelling | word emitted at | how the use loads it | checker |
| --- | --- | --- | --- |
| plain literal | at the use | `lfs f,0(r2)` via SDA21 | silent |
| `static const f32 x = V;` | **nowhere** -- folded *and* dead-stripped, section size unchanged | literal at the use | silent |
| `const f32 x = V;` (external linkage) | declaration point **and** a duplicate at the use | the duplicate | silent |
| `static const f32 x[1] = {V};` | declaration point, offset 0 | `lfs f,0(r2)` via SDA21 | **gated** |
| `static const struct { f32 v; } x = {V};` | declaration point, offset 0 | `lfs f,0(r2)` via SDA21 | silent |
| `static const union { f32 f; } x = {V};` | declaration point, offset 0 | `lfs f,0(r2)` via SDA21 | silent |
| named one-member `struct` typedef | declaration point, offset 0 | `lfs f,0(r2)` via SDA21 | silent |

The four aggregate rows produce the same object file byte for byte. The checker does not see
three of them because `RE_ONE_ELEM` keys on the literal `[1]` subscript and `RE_LBL_UNION` only
on an `lbl_`-named union. Re-deriving section 12's three units with the struct spelling
reproduces its figures to the digit: `matched_data` 1203321 -> 1203621 (+300), `complete_units`
906 -> 909, `fuzzy_match_percent` 99.81558 unchanged, every section of `main/rcp_dolphin`,
`279_AppleOnTree` and `701` at 100.0, `all_source` EXIT=0 -- and `banned_shapes_check` at
102 hits / 83 baseline / 19 regrowth, **unchanged, adding zero**.

**Verdict: there is no clean spelling, and none of these was landed.** A one-member struct or
union read only through that member is the banned construct in different syntax: its only
function is to stop MWCC folding a scalar `const`, which is exactly what the check's own
rationale says the one-element array is for. Passing the gate on a regex technicality is not a
recovery, and no 2002 developer writing "a named constant zero" reaches for a struct wrapper.
The verified array-spelling patch stays parked at `/private/tmp/A68_declared_consts.patch`; the
decision to accept any of these forms into `tools/banned_shapes_baseline.txt` belongs to whoever
owns that baseline, and it is now a decision about a shape rather than about a spelling.

### 12c. Why a genuine multi-word aggregate cannot stand in for it

The one honest aggregate is a table whose words are all real and adjacent in retail's pool, so
the obvious escape is a two-element array with two honestly-named words. It is closed on the
code side, not on the naming side. Measured on `701` with
`static const f32 sAndrossHandProgressRange[2] = {0.0f, 1.0f};` covering both the zeros and the
ones: `.sdata2` comes out **byte-identical** -- the pair lands at 0x0c with both words in
retail's order -- but `.text` breaks at every read of element `[1]`, which compiles to

    li      r8,0            ; R_PPC_EMB_SDA21 sAndrossHandProgressRange
    lfs     f1,4(r8)

where retail has the single `lfs f1,0(r2)` against its own symbol. Retail loads every one of
these words at offset zero from its own symbol, so a member at offset 4 is the wrong shape by
construction. Section 12 predicted this cost as `addi rN,r2,sym`; the emitted form is the base
materialised into a GPR by the SDA21 relocation, and the conclusion is the stronger one: **an
aggregate places only its first word for free**, so every mover word needs its own four-byte
symbol, and a four-byte aggregate holding one `f32` is the banned shape whatever brackets or
braces it wears.

That also settles the three rows individually, because each needs at least one *lone* word that
no honest pair can carry: `main/rcp_dolphin` wants `2.146452f, 2.520326f, 255.0f` and a
three-word array is twelve bytes, which goes to `.rodata`; `279_AppleOnTree` wants a lone `1.0f`
ahead of the fall-scale pair `{0.25f, 0.75f}`; `701` wants a lone `0.0f`, and the only pair that
would cover it is `{0.0f, 1.0f}`, whose members are a move start time, four velocity components,
a render scale and a progress limit -- no honest name spans them.

### 12d. The intra-function frontier is complete, and every row on it is adjudicated

Section 8b's classifier -- map each pool slot to the function of its first reference on both
sides -- was applied to all 31 sub-100 data sections at `f63cb3dc08`, extending its list of six
named specimens to the whole frontier. The 31 rows split 18 intra-function against 13
cross-function:

- **intra-function (8b), 15 with a single owning function**: `main/vecmath`
  (`interpolate`), `main/rcp_dolphin` (`Rcp_InitDistortionEffects`), `dlls/engine/5`
  (`renderSunAndMoon`), `dlls/engine/7` (`lightningDrawStrand`), `dlls/engine/19`
  (`waterfx_drawSplashBurst`), `dlls/engine/24` (`boneParticleEffect_update`), `dlls/engine/68`
  (`firstPersonDoControls`), `dlls/engine/69` (`CameraModeTalk_update`), `202/mikaladon`,
  `260_SmallBasket` (`SmallBasket_spawnContents`), `328_CFGuardian` (`cfguardian_steerToward`),
  `362_CRrockfall` (`crrockfall_update`), `429_SH_thorntai`, `609_DR_LaserCan`
  (`drlasercannon_aimAtTarget`), `main/trig` (`fsin16Approx`). Three more are degenerate:
  `300_Transporter` and `musyx/sal_volume`'s two `extab` sections have no owning function at all.
- **cross-function (8), 13 rows**: `main/model`, `main/object`, `main/objhits`,
  `main/pi_videoinit`, `main/shader_dolphin`, `main/newshadows`, `main/acosf`,
  `track/intersect_render`, `dlls/engine/0`, `704`, `279_AppleOnTree`, `701`, `332`.

One refinement the sweep forced: 8b's test ("same owning function on both sides") is necessary
but not sufficient. `main/rcp_dolphin` passes it -- all three moved words are
`Rcp_InitDistortionEffects`'s on both sides -- yet it is a section 12 mover, not an 8b row,
because retail puts them at the pool *head*, ahead of four other functions' words, and
`Rcp_InitDistortionEffects` is the last function in the unit. The test has to be read together
with the destination: an 8b row is one whose moved words stay inside their own function's
contiguous run.

No unadjudicated row survives: section 8b prices `engine/19`, `engine/69`, `202/mikaladon`,
`429_SH_thorntai`, `609_DR_LaserCan` and `main/trig`; section 9 prices `engine/5` (its lone
missing word is the `gSkySunMoonRiseScale` crutch, and defining it costs `renderSunAndMoon`
99.476 -> 98.214 for +176 data, a net tree loss) and `engine/68`; section 8's table declines
`engine/7`; section 12 covers `main/vecmath`, `engine/24`, `328_CFGuardian`, `362_CRrockfall`
and the movers; `300_Transporter` was retired as a padding artifact in section 11; and
`musyx/sal_volume`'s two rows are zero-size sections. **Mission-2-style pure statement or use
motion reaches none of them** -- confirming 8b's finding from the other direction, since every
intra-function row's mint order is only reachable through a live use and a live use is what
moves the load.

## 13. The cross-TU declaration laws (measured 2026-08-02/03)

Sections 9-12 price what a declaration *places*. This section records what a declaration *costs*
when the same object is spelled two different ways in two translation units. MWCC type-checks one
TU at a time, so an `extern` that contradicts its definition reaches no diagnostic and no gate;
`tools/extern_type_census.py` finds them by comparing every file-scope declaration of every symbol
across `src/` and `include/`. Every rule below was established by rebuilding and comparing the md5
of **all 1013 source objects**, so each is exact rather than inferred.

### The defining TU proposes, the reading TU disposes

A disagreement is a defect, not a style choice: one side is wrong, and which side is wrong is
decided by the emitted code, not by seniority. The definition is the proposal; the reader's
opcodes are the adjudication. Three tells settle nearly every row:

- **signedness** — a compare feeding a branch emits `cmpwi` for `int`/`long` and `cmplwi` for
  unsigned, so the reader's compare names the type. `gForceLoadImmediately` was defined `u32` and
  read signed by `pi_dolphin.c`; the *definition* changed. Same tell moved `gSaveCardState` to
  `volatile s32`, `sMapFileNameIndexRemapTable` to `int[]`, and the two map-block draw-order
  tables to `s8[16]`.
- **`volatile`** — `gAssetLoadCompletedFlags` is polled across an asset load, and the polling loop
  prices both halves of its type exactly: dropping `volatile` costs `initLoadFiles`
  100.0 -> 97.565, and `volatile u32` instead of `volatile int` costs 100.0 -> 98.756. The
  definition becomes `volatile int` and `objprint_dolphin.c`'s plain `u32` goes.
- **what the address is actually made of** — `gMapsTab` does integer arithmetic, so typing it as a
  pointer *changes `shader.o`* and `int` is the object; `gMapBlockCellStateTables` has
  `&...[4]` taken into an `int*`, so its twenty bytes are five `int`s, not `u8[0x14]`; `gTrkBlkTab`
  is read as halfwords, so it is `u16*`, not `void*`.

`d658fa7be5` settled six rows this way, `96ef150510` fifteen more, `1b59bb079c` six of a further
sixteen, `8722b792b1` `gGlowLightList` (`u8[0x190]` held against `ModelLightStruct*[100]` read —
400 bytes is 100 pointers). Every one of those commits is md5-identical over all 1013 objects: a
type disagreement that is genuinely a spelling costs nothing to settle, and one that is not shows
up as a changed object immediately.

At `cb88387945` the census reports 47 residual rows. Most are the tool's own spelling blindness
(a typedef against its `struct` tag: `GXData*` vs `struct __GXData_struct*`,
`FrontendSaveSlot*` vs `struct FrontendSaveSlot*`). The rest are the adjudicated pins —
`renderFlags` (`int` in `tex_dolphin` only), `gLightmapDrawQueue`, `sNAttractModeStringBlock`,
`gAttractMoviePrepareReadyQueue`, `gTexIndMtxTable`, `gDll12Interface`, `aramNormalPriorityQueue`,
`gSceneCamera`, `gGlowLightList` — which are load-bearing or priced below. **Do not "fix" a pin.**

### Type visibility, not the include, is the relocation constraint

Moving a declaration out of a `.c` and into a header the reader already includes is free by
construction — the token stream each TU sees is unchanged. It fails for exactly two reasons: the
symbol is already declared there as something else (settle it first, above), or **the header cannot
name the type**. Fifteen declarations were stranded on the second reason alone — the target header
could not see `ModelLightStruct`, `OSMessageQueue`, `GXColor`, `AIDCallback`, `OSThread`,
`GXFifoObj`, `Camera` or `EnvironmentUpdateInterface`.

Gaining that visibility is a *per-object* question, and the answer is not uniform:

- **Free (measured, md5-identical over all 1013):** `pi_dolphin.h += GXFifo.h, OSThread.h,
  OSStopwatch.h`, which carries `gGxFifoObj`, `gVideoWaitThread` and `gFrameStopwatch` out of
  `pi_videoinit.c`; `attract_movie_api.h += dolphin/ai.h` for
  `gAttractMovieAudioPrevDmaCallback`. Also free: adding the *owning* include to a `.c`
  (`mm.c`, `engine/52`, `engine/0`, `shader.c`, `MWTrace.c`, `dll_80136a40.c`), each measured on
  its own object first.
- **Priced by class #70 (rejected):** `shader_api.h += main/model_light.h` rewrites **33 DLL
  objects**. Every section's *content* is byte-identical and no score moves, but the local
  literal-pool symbols renumber — `@103 -> @105` at the same address, in the same section, with
  the same size. There is no evidence the new numbering is nearer retail's, so the three light
  lists stay declared where they are. Same verdict for `textrender_internal.h += GXStruct.h`
  (`subtitle.o`) and `shader_api.h += camera.h` (4 DLL objects).

So the rule is: propose the include, then **measure the object, not the tree** — a score-flat,
content-identical rewrite of 33 objects is exactly the shape that a score gate reports as zero
and that md5-of-every-`.o` reports as 33.

### An extern array stays UNSIZED — and so, sometimes, does the definition

Completing an array type is a semantic change, not a declaration move, and it is priced on both
sides:

- **Declaration side.** `gAttractMovieAudioDmaBuffer` moves into `attract_movie_api.h`, but the
  declaration must stay `extern char gAttractMovieAudioDmaBuffer[];`. Carrying the `[0x50C]` into
  the header completes the type for `dll_3e.c` and costs `prepareAttractMode` 100.0 -> 99.087.
- **Definition side, and it is the data axis.** `2589a58b75`: `projgfx/194`'s
  `sProjdfp1rDoNoLongerSupported` was defined `char [40]` where the string is retail's 0x24 bytes.
  The four surplus bytes over-size the object, and because `matched_data` is all-or-nothing per
  section (§8), the *whole* `.data` section leaves `matched_data` even though every byte already
  matched. Declaring it `char []` and letting the initialiser size it: data 0/72 -> 72/72, unit
  100/100, `matched_data` +72 at zero tree regression. Its twenty sibling `OSReport` strings are
  all `[]` and all 0x24; `194` was the lone outlier.

The mirror rule for qualifiers: `const` on a *definition* is a section decision, not decoration.
`sReverbStdDelayLengths` is **not** `const` — making the definition `const` moves it out of
`.data` and costs 16 bytes of `matched_data`, so `1b59bb079c` moved the reader's `const` instead.

### The zero-build census, and what it is allowed to conclude

`b50bfac4ff`, `e133454ffd` and `cb88387945` extend the same method past objects to the other
declaration kinds — prototypes, macros, struct tags — where a disagreement can be found by reading
the tree rather than by building it: an `#ifdef` selecting between two prototypes for one function
where the macro is never defined anywhere (`DLL_0126_TRIGGER_LEGCODE_INT`,
`FRONT_GAMETEXTBOX_NARROW`, `OBJECT_RENDER_LEGACY_DIRECT_CALL`, one dead branch each); a struct tag
whose member is `m` in the header and `v` in a local copy (`IndTexMtx23`); a fabricated
`#define NAN 0.0f` in `ghidra_import.h` colliding with `math.h` for ~180 files, plus the bare
`#undef NAN` written to work around it; a header included by nothing whose five prototypes are all
contradicted elsewhere (`MSL_Common/math_ppc.h`, including a one-argument `atan2(double)`); and 99
local `#define`s shadowing an identical-valued definition in a header the file already reaches.

The census tells you a hunk is byte-identical **by construction**, and that claim is worth
believing only because it was checked: all 1013 objects came out md5-unchanged in each case, and
the 18 macros a file genuinely needs (`PAD_BUTTON_*`, `RENDERFLAG_*`, `TEXT_CTRL_*`, `*_OBJGROUP`)
were kept rather than assumed redundant. "Byte-identical by construction" earns the full gate, not
an exemption from it.

## 14. Re-sweep of the sub-100 code frontier against the post-convergence laws (measured 2026-08-03)

The code axis was declared converged before most of the current law set existed. This section is
the one re-opening, run at `97746b6bd3` with the `A + -C`, per-symbol and source-text-order lenses.
It reports a partition, two emptiness results and two re-priced rows. The verdict: **convergence
stands** — every row the new lenses reach was already banked, and the two whose mechanism the new
lenses genuinely corrected are still priced.

### The partition, and why it is worth having

`tools/a71_mnhist_scan.py` compares a function's normalised target and current instruction streams
three ways instead of one. That splits every sub-100 row into exactly three kinds:

| Kind | Test | Rows |
|---|---|---|
| colouring | opcode SEQUENCE identical, registers differ | 136 |
| order | opcode MULTISET identical, sequence differs | 10 |
| operation | opcode multiset differs | 67 |

There is no fourth kind, and no row is reloc-only. The test is cheap, needs no source, and answers
the question that decides whether a row is worth opening at all: a colouring row is #108/#82 by
construction and no source edit reaches it, whereas an operation row is asking the code generator
for something the source text chose. Of the 67 operation rows, 5 are the never-touch islands
(the three `ObjModel_Transform*`, `zlbDecompress`, `modelApplyBoneTransform`), 13 are the closed
zero-weld `li`-vs-`mr` cap of §5, and the rest are the already-banked §1-§4b rows. Exactly two of
the 213 had no prior mechanism on record, both `li`-vs-`mr`.

### Emptiness result 1: the sign-in-the-constant class does not generalise

`tools/a71_signscan.py` reads the f32 and f64 words of every unit's pool sections on both sides and
reports any value one side holds at the other's opposite sign. Over all 1050 units the answer is
**zero**. The opcode partition agrees from the other direction: no row outside the PS islands emits
`fnmsubs`/`fnmadds`/`fsubs` where retail emits `fmadds`/`fmsubs`/`fadds`. `trig` was the whole
class, and `e173a2c951` closed it.

### Emptiness result 2: the load-order flip has one live site, and it is priced

`tools/invcmp_scan.py` finds three functions; two are never-touch islands. The third is
`dlls/engine/6` `sky2_run`, and its banked description — "the ENTIRE residual is ONE `fneg`/`lfs`
schedule swap, 8 spellings inert" — is **refuted**. The residual is an operand-order row: retail
compares `best.x` against a once-loaded zero and skips with `ble-`, we load the zero first and skip
with `bge-`. Rewriting `if (zero < best.x)` as `if (best.x > zero)` reproduces retail's compare
exactly, both `fcmpo`/`ble-` pairs collapse into the matched region — and the surrounding scratch
FPRs rotate f1<->f2 across ten `fmadds`, so the row measures **99.65298 -> 99.49717**. Dropping the
`zero` temp for a bare `0.0f` gives the identical 99.49717 (MWCC interns the load either way), and
hoisting the `zero = 0.0f;` initialiser above the search loop gives the identical 99.49717 again —
which is the statement-order law stated as a measurement: **a dead initialiser move is inert.**
The row is priced by #82, not by the operator.

### The ternary's arm order is real, reachable, and still priced

`454_DIMCannon` `DIMCannon_updateAim` is the same story on the order axis. Retail's clamp emits
`ble-` to the assignment plus a `b` over it and leaves the result in the variable's own saved `f31`;
ours emits `bge-` and lands the result in scratch `f3`. Retail's spelling is the one its own sibling
two lines later already uses, `(distSq > 10.0f) ? distSq : 10.0f`, and writing it that way does make
the `ble-` match — but MWCC then folds the `b` away and still targets `f3`, so the whole downstream
chain diverges: **99.76923 -> 99.28205**. The in-place `if (distSq < 10.0f) { distSq = 10.0f; }`
keeps the variable in `f31` and costs more still, **98.82051**. Both spellings are more faithful
readings of the target than the baseline and both lose; the baseline stays.

### What this says about the frontier

Three of the new laws fired on real rows and none of them paid. The pattern in all three is the
same and is worth stating once: **the new lenses locate mechanisms the old ones could not name, and
the located mechanism is downstream of a register decision that no source edit reaches.** A row
whose residual survives a correct operator is a colouring row wearing an operator's clothes. The
136 colouring rows never needed re-opening, the 10 order rows are all §2/§3/§6-banked, and the 67
operation rows are the banked §1-§4b set plus two `li`-vs-`mr`. Re-open a row only when a lens finds
a mechanism *and* the mechanism's fix does not have to survive an allocator.

## 15. The ten order rows, worked one at a time (measured 2026-08-03)

§14 partitioned the sub-100 code frontier into 136 colouring / 10 order / 67 operation rows and
left the order bucket described as "all §2/§3/§6-banked". This section opens all ten, because the
order bucket is the only one whose defining property — the same instructions in a different
sequence — names a lever the campaign actually has: source-text order. Every row below was worked
with the text axis (statement order, operand order, expression grouping, temp naming, declaration
placement, loop shape) and each gets a verdict.

### The measurement that reframes the bucket

Diff the two streams with the operands thrown away and the order rows stop looking like a kind:

| Row | Unit | Score | Mnemonic-sequence delta |
|---|---|---|---|
| renderShadows | main/newshadows | 99.715 | 1 instruction slid |
| mmpMoonRock_update | 386_MMP_moonroc | 99.516 | 1 |
| gameTextFinalizeLoad | main/textrender_run | 99.334 | 1 |
| boxBlurTexture | main/newshadows | 99.167 | 1 |
| wispBaddieProcessAnimEvent | 202/sharpclaw | 99.156 | 1 |
| Checkpoint_buildControlPoints | engine/3 | 98.464 | 1 |
| playerUpdate | 195_Player | 98.432 | 1 |
| debugTextDrawToFrameBuffer | main/dll_80136a40 | 97.656 | 1 |
| staffUpdateSegmentTransforms | main/objprint | 97.018 | 2 |
| expgfxGetSlot | engine/10_expgfx | 95.899 | 2 (the same slide at two sites) |

**Every order row is one — at most two — instructions out of place, sitting inside a register
permutation that accounts for the rest of its gap.** `boxBlurTexture` is the extreme case: 1356
instructions, one slid `clrlwi`, and 115 diff hunks of which 114 are `r26`<->`r28` / `r30`<->`r31`.
So the order bucket is **not a third kind of divergence**. It is the colouring mass plus a slid
instruction, and the honest reading of §14's partition is that the three-way test is really a
two-way one with a one-instruction tolerance band. Keep the test — the slide is exactly where a
source-text mechanism can hide — but do not read "order" as "reachable".

### The one that paid

`main/newshadows` `renderShadows`. The elevation angle is `sqrtf(sqA + sqB)`; retail emits
`fmuls f0,f22,f22` (the X term) before `fmuls f1,f21,f21` (the Z term), and we named the squares
the other way round, so both products and the `fadds` that consumes them came out in the wrong
order. Swapping the two declarations is the whole fix: **99.69954 -> 99.71494** (`9b4d9f45b9`).
Two things were priced on the same row: collapsing the temps into an inline
`sqrtf(vAx*vAx + vAz*vAz)` costs **99.48382**, so the named temps are load-bearing; and the
`castSlot` address wants retail's `add base,index` operand order, but *every* spelling that writes
the base on the left of the sum — including the natural `&shadowData->castSlots[i]` — makes MWCC
group the constant with the scaled index instead and costs **99.40678**.

### The nine that did not, and what stopped each

- **`mmpMoonRock_update`** — a real structural row, and the closest any of the ten came. Retail
  materialises the "no conflict" flag *in the loop's exit block* (`li r0,1` between the back edge
  and the join) and the `break` path jumps over it, so the flag never crosses a call and never
  needs a saved register; we hoist `li r26,1` into the preheader and pay a callee-saved home. The
  shape retail has is what a `goto` past the exit assignment emits, and `goto` is banned. Two
  non-`goto` spellings measured: moving the declaration to function scope **99.51613 -> 99.36636**,
  and rewriting the loop as `while (1)` with the exit test and its `spacingClear = 1` inside the
  body **-> 98.23733**. Moving the declaration into the inner block does not compile — the flag is
  read after that block closes. PRICED.
- **`gameTextFinalizeLoad`** — retail computes `(stringTable + numStrings*4) + 4`, we compute
  `(numStrings*4 + 4) + stringTable`. This is a front-end canonicalisation, not a source choice:
  `&stringTable->offsets[n]`, `&strs[n]` and an explicit `(u8*)stringTable + n*sizeof(int) +
  sizeof(int)` all produce the **same object byte for byte**. MWCC always folds the constant into
  the scaled index first. PRICED.
- **`boxBlurTexture`** — one `clrlwi` (the `u16 fillHalfword = fill;` truncation) that our
  scheduler emits two slots earlier than retail, inside a 114-hunk register permutation.
  COLOURING.
- **`wispBaddieProcessAnimEvent`** — retail materialises the `0` for `activeEventIndex = 0` in
  `r0` immediately before its `stb`; we allocate it to `r4` and the scheduler then hoists it eight
  instructions into a load-delay slot. The statement order already matches retail. COLOURING.
- **`Checkpoint_buildControlPoints`** — the slide is one `lfd` in a LICM preheader: retail hoists
  the s32->double bias, then pi, then 32768.0f, then 2.0f (exactly the loop body's first-use
  order), and we hoist 2.0f first and the bias third. The other 73 hunks are a parameter-home
  permutation. COLOURING.
- **`playerUpdate`** *(195_Player, owner-hot under C73 — analysed read-only, not edited)* — an
  argument-setup `mr` that retail emits seven instructions earlier, inside an `r29`<->`r30` swap
  that accounts for essentially the whole 1.57 gap. COLOURING.
- **`debugTextDrawToFrameBuffer`** — retail schedules the `mr r26,r5` parameter home into the
  `addi`/`mulli` dependency gap; we emit it after both `mulli`s. Underneath, retail gives the `x`
  parameter `r29` and puts `row1`/`row0` in `r27`/`r28`, while we give `row1`/`row0` `r29`/`r28`
  and push `x` down to `r27`. Swapping the `row1`/`row0` declarations does move the pair (to
  `r29`/`r28` in retail's relative order) but never lifts the parameter above them:
  **97.65625 -> 97.39584**. The target colouring is not reachable from the declaration list
  because the register that has to move belongs to a parameter. COLOURING.
- **`staffUpdateSegmentTransforms`** — two slides. Inlining the `joint` temp into its
  `ObjModel_GetJointMatrix` call is **exactly byte-neutral** (97.01807 both ways), and writing the
  second site's matrix address in retail's addend order (`*(int*)(...) + idx2 * 0x40`) costs
  **97.01807 -> 92.28915**. PRICED.
- **`expgfxGetSlot`** *(engine/10_expgfx, owner-hot under C73 — analysed read-only, not edited)* —
  retail materialises the `1` of `1u << slotIndex` before loading the active mask; we load first.
  The same slide appears at both unrolled sites. PRICED pending an owner with the file.

### What this adds to §14's closing rule

§14 said to re-open a row only when a lens finds a mechanism *and* the fix does not have to survive
an allocator. The ten order rows sharpen that into a test you can apply before spending a build:
**an order row is worth opening only when the slid instruction is the one the source text names.**
`renderShadows` slid an `fmuls` whose operand the source picks by writing one square before the
other — reachable, and it paid. The other nine slid a parameter-home `mr`, a rematerialised `li`,
a LICM hoist or a scheduler's load-delay filler — none of which any source text names, and none of
which paid. A dead-initialiser move stays inert (§14), a canonicalised address expression stays
canonical, and a constant that retail rematerialises where we colour is a colouring row wearing
order's clothes.

## 16. The symbol-identity oracle: what the carve's symbol table says that no score reads (measured 2026-08-02)

§11 measures that `.bss` layout is set by first-use order and that declaration order is
completely inert. That makes the retail carve's `.bss` order a free oracle for the source
text's use order — and nothing in the tree was reading it. `.bss` has no bytes, so objdiff's
data score cannot see a permutation; md5-of-every-`.o` cannot see one either when the unit is
not linked, which the carve-linked units are not. `tools/bss_order_scan.py` reads it, on both
axes, over all 1013 source objects against their carve.

**Order.** 21 sections place their shared symbols in an order the carve does not. Every one is
`.bss`. Each row is a claim that our TU touches those objects in the wrong order: a missing use,
a use spelled against the wrong object, or a wrong TU boundary. The specimen worked here is
`main/tex_dolphin`: ours is `gViewFrustumPlanes, gPlayerRelativeFrustumPlanes, gRcpPendingWarpDest`
and the carve is `gPlayerRelativeFrustumPlanes, gViewFrustumPlanes, gRcpPendingWarpDest`.
Swapping the two declarations is inert exactly as §11 predicts, and both frustum relocations in
`tex_dolphin.o` already agree with the carve's — mnemonic, register and symbol — so the source
text is missing whatever makes the player-relative array first. The carve's naming is not the
suspect: retail's own `buildPlayerRelativeFrustumPlanes` materialises `0x803878D8` (`lis r3,0x8038;
addi r31,r3,0x78d8` at `0x8005aadc`, read out of `main.dol`), which is the carve's offset 0, so
the address-to-role assignment in `config/GSAE01/symbols.txt` is right and the ordering defect
is ours.

**Identity.** 109 symbols carry a different name than the carve gives the same
section/offset/size. Two kinds, and only one of them is debt:

* *naming debt in the map* — ours is a recovered name, the carve's is still `lbl_`/`gap_`. 76 of
  the 89 same-size rows. Score-free either way; it means a rename landed one-sided.
* *a contradiction* — both sides carry a real name and they disagree. `main/tex_dolphin` and
  `main/subtitle` each transpose a pair (`gSubtitleLineTimes` <-> `gSubtitleLineTable`);
  `main/model`'s `.sbss` is shifted one slot by a leading static; `dolphin/os/OS.o` calls
  `__OSInIPL` `AreWeInitialized`.

**The `lbl_` direction is itself an oracle.** Of 1013 objects exactly ONE has the divergence the
other way — our source saying `lbl_` where the carve has a recovered name — and it is the whole
of the `LBL_CONST_DEF` question: `src/dlls/objects/597/597.c`'s `const GXColor lbl_803E5AE0 =
{5, 5, 5, 5};` is the object `config/GSAE01/symbols.txt` already calls
`sSnowBikePathPointParams` at `0x803E5AE0`. The name is stale source from a half-landed rename,
not a pool-forcing const, and the two readings price as follows:

| Reading | Tree | 597 unit | `.sdata2` word 0 |
|---|---|---|---|
| baseline | 99.81598 / 2515196 / 1203321 | 99.90702, data 896/896 | `05050505` |
| rename to `sSnowBikePathPointParams` | 99.81598 / 2515196 / 1203321 | 99.90702, data 896/896 | `05050505` |
| delete, spelling it `GXColor pathParam = {5,5,5,5};` at the use | 99.81542 / 2513832 / **1202925** | **99.81951**, data **500**/896 | **gone** |

All five section digests of `597.o` are byte-identical across the rename; only the symbol string
moves. Deleting it costs the unit's entire 396-byte `.sdata2` — the pool slides down four bytes —
plus 1364 bytes of matched code, because `05050505` is an aggregate no float literal mints and
only a file-scope definition can place. It is read (`SnowBike_init` copies it into the `GXColor`
it hands to the path interface's `setup`), it is 100% of the carve, and it is a struct, which
`banned_shapes_check`'s own docstring already exempts — `RE_LBL_SCALAR` catches it only because
the regex excludes `union` and `[` and nothing else. Three of the other five `LBL_CONST_DEF`
baseline rows are the same misclassification (`engine/7`'s three `const SnowVec3`, each read into
a different destination, each a distinct 12-byte `.rodata` object); only `engine/21`'s and
`Baddie`'s `const f32 lbl_... = 0.0f;` are the scalar shape the ban was written for.

## 17. Where `.bss` allocates, and why it is worth `complete_units` (measured 2026-08-02)

Section 11 measures that `.bss` layout is first-use order and that declaration order is inert.
Both halves are true and neither is the whole law: `main/mm.c` defines its three `.bss` objects
at lines 360/372/374 after `extern` declarations at 181/302, uses them first at 193/315/522, and
lays them out in **definition** order — which no reading of "first-use order" produces, and which
survives swapping the two definitions unchanged.

### The law

Measured with a six-case probe battery compiled through the real command line (both live flag
sets agree; `/private/tmp/a74probe/`):

| probe | layout |
|---|---|
| definitions `A B C` at the top, uses `A B C` in three functions | `A B C` |
| definitions `A B C`, uses `C B A` | `C B A` |
| externs at the top, uses `A B C`, definitions `A B C` at the bottom | `C B A` |
| each definition placed immediately *after* its own use | `C B A` |
| `A` used then defined; `B`, `C` defined then used | `B C A` |
| `sizeof(B)` before the uses; separately, `int* pB = B;` before the uses | `A B C` — neither allocates |
| a use of `B` inside an **uncalled static function** above the others | `B A C` |

So: an object is allocated at the **first use that follows its definition**; an object with no
such use is allocated at end of translation unit, and those come out in **reverse definition
order**, after every allocated one. The front end's position is what counts, not the code
generator's — an uncalled (or inlined) static's body allocates where it is written, while
`sizeof` and a `.data` pointer initialiser do not allocate at all. Section 11's intra-expression
depth rule survives unchanged (`A[1]+B[2]+C[3]` gives `A C B`).

### It is not a compiler-version artifact

83 source objects share two or more `.bss` symbols with the carve; 62 already agree. Every other
section agrees everywhere (`.data` 290/290, `.sbss` 159/159, `.sdata` 123/123, `.sdata2` 32/32,
`.rodata` 28/28). Among the units that agree, `main/thp/THPRead.o` and `main/audio_stream.o` are
the specimens: their queue objects are declared `extern` where the types are, defined at the
bottom of the file, and come out in reverse definition order — the law's second clause,
reproduced by our compiler and matching retail. The tree already carries the shape; it was simply
not carried where it was needed.

### What the 21 rows were, and what closed them

Each of `tools/bss_order_scan.py`'s 21 rows is a TU that defines the objects above the code
instead of below it. Moving the smallest sufficient set of definitions past their last use (an
`extern` left behind only where no header already declares the object) reproduces the carve's
order **exactly** in all 21, from one moved definition (`track_dolphin`, `model`, `53`, `203`,
`DBstealerwo`) to seventeen (`engine/0`). `engine/2` needed no move at all: its fifteen
definitions already sit at the bottom of the file in the carve's order, which is precisely
backwards — reversing that block put all fifteen where the carve has them. Every edit is
byte-neutral by construction: `objdump -s` over all 1013 objects against a pristine build of the
parent gives 0 differ / 0 missing / 0 new.

### The payoff, and the control that proves it

A NonMatching unit is linked from the retail carve, so its `.bss` order cannot reach the DOL —
which is why this axis has no score. A **Matching** unit is linked from our own object, and a
permuted `.bss` moves every DOL word that references it. `main/objlib` is the control: at
100.00000 fuzzy it stayed NonMatching for exactly this reason, and it is now measured, not
inferred — with the parent's `objlib.c` and `Object(Matching, ...)` the forced link **FAILS** the
sha1; with the two definitions moved it passes.

  objlib, textrender_drawbox, thp/dll_3b, objects/203   NonMatching -> Matching
  complete_units 910 -> 914 of 1050, tree 99.81614 / 2517640 / 1203321 flat

`main/audio.c` is the fifth 100.00000 candidate and does not flip: its `.bss` order is fixed too,
but our `.data` is `0xe6b` bytes against the carve's `0xe70`. The carve carries a five-byte
`gap_07_802C5D73_data` after `sMidiWadPath` that our last string does not produce, and
`section_alignments` does not reach it — that tail is the remaining blocker, not the `.bss`.

### `.sbss` is the same law with the uses removed

`.sbss` layout is reverse declaration order, and 159 of 159 units already agree — but a `static`
declared *below* the file's `.sbss` block takes slot 0 and pushes every global down one word.
`main/model.c` was the one instance: `sGQR7Config` at line 100 against the block at 32-40. The
carve puts a 4-byte `gap_10_803DCB6C_sbss` — an object dtk cannot name because it is a local — at
the end, which is that static in retail's build. Declaring it first puts it last and lands all
nine globals on the carve's offsets (name divergences 105 -> 96; the one row left in that unit is
the unnameable local itself).

### What is left on the axis

`bss_order_scan` is at **0** order divergences and **96** name divergences. Of those 96, 64 are
`.sdata2` and 28 are `ours=<a local static or an unrecovered lbl_> vs retail=gap_*` — benign by
construction, since dtk cannot name a local. The inverse sweep asked for by §16 (we name it,
the carve says `lbl_`) is **68 rows and is the const-recovery lane's**: they are its pool anchors
(`gTumbleweedBushRenderScale`, `gDFropenodeOneHundredth`, `sFireFlyDespawnDelay`, ...), the same
population as `banned_shapes_check`'s 19 `SINGLE_ELEM_CONST_ARRAY` regrowth rows, and not a
naming defect to be "fixed" by another lane. Only **two** rows are the §16 direction — `597.c`'s
`lbl_803E5AE0` (owner-hot) and `model.o`'s `.sbss`, closed above.

