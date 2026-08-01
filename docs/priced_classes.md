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

## Related recurring REGRESSION class (fixable — not priced, listed so windows get scanned)

Pool-const purge/retune commits historically gated on matched_code only and twice shipped
per-fn regressions (fcmpo operand-order flips: sky2_run via `zero < best.x` -> `best.x > zero`,
fixed in `2c32711828`; DIMCannon_updateAim sibling) and once shipped matched_data -4936
(engine/0 purge, retuned in `f596800ffa`). Delta-scan every merge window with per-fn fuzzy
AND matched_data; an fcmpo flip is repaired by restoring the retail operand order in source.
