# The data axis — CLOSED

`matched_data` measures data-**symbol pairing**, not pool contents. Every remaining shortfall in
the tree has been classified, and none of it is a missing constant. This file exists so the axis
is not re-mined: read it before spending a build on a data score.

The axis produced exactly **one** real content recovery (`player_SeqFn`, +8232 bytes, a retype
of `obj->extra`). Everything else measured as a tooling artifact or a TU-boundary artifact.

---

## 1. The law: pool-pairing blindness is SECTION-granular

objdiff pairs data symbols **by name**. Our compiler emits pool constants as anonymous `@N`
symbols; the splitter carves the retail side as named `lbl_8XXXXXXX`. Those can never pair.

The sharpened form — and the part that was not previously written down:

> **A data section containing ANY unpairable anonymous symbol scores ZERO as a unit,**
> **regardless of how many of its named symbols are byte-perfect and name-identical.**

This is why partial data scores are rare: sections tend to be all-or-nothing.

### Worked proof — `track/intersect_render.c`

`total_data 452 = .rodata 216 + .sdata2 236`. `.rodata` scores **216/216**; `.sdata2` scores
**0/236**. The single variable that separates them:

| section | named syms | anonymous `@N` | score |
|---|---|---|---|
| `.rodata` | 2 | **0** | 216/216 (100%) |
| `.sdata2` | 13 | **40** | 0/236 (0%) |

Byte-level decomposition of the 236, comparing our object against the retail carve:

| bytes | population | status |
|---|---|---|
| **51** | 13 constants our source already names (`sApertureColorBlack`, `sDistortKColor0/1/2`, `sColorFilterKColor0/1/2`, `sMoonFxTint`, …) | byte-identical **and** name-identical on both sides — **still scores 0** |
| **76** | 18 anonymous `@N` pool floats vs the splitter's `lbl_803DEExx` | byte-identical, unpairable by construction |
| **109** | 27 entries where pool **order** diverges: doubles land at different offsets (size 4v8 / 8v4), and retail holds 5 entries at `+0x5c/0x6c/0x6e/0x8c/0x94` we never emit | genuine divergence — TU-boundary artifact, do not reconstruct |

**127 of 236 bytes are already byte-correct and score zero purely for pairing reasons.** The 51-byte
row is the exhibit: identical name, identical size, identical bytes, zero score.

An earlier, weaker proof of the same law: `engine/5` had three genuinely missing `.sdata2`
constants (`0.55f`, `10800.0f`, `86400.0f`). Replacing the stand-ins with literals made the pool
contents *exactly* match — 44 slots / 41 distinct on both sides, nothing absent either way.
**`matched_data` did not move: 600/776 before, 600/776 after.**

---

## 2. Two gates that were tried and refuted — do not re-try either

### Naming is not the gate (measured null)

All 12 `lbl_803DEExx` in `intersect_render`'s `.sdata2` were renamed to the identifiers our source
already used. The retail object was confirmed **re-carved** with the new names
(`sApertureColorBlack`, global, 4 B at `+0`), and `report.json` was rebuilt fresh — the first run
read a **stale** report and would have been read as a null for the wrong reason.

Result: `matched_data` stayed at exactly **216/452**. Naming a byte-identical constant on both
sides buys nothing while anonymous symbols remain in the section.

### Retail-side binding carries ZERO information (splitter artifact)

The tempting second hypothesis is `static`: retail `.sdata2` is 57 global / 1 local, ours is
0 global / 54 local. It is wrong twice over.

1. `.rodata` scores **216/216 with the same mismatch** — our two symbols are `l`, retail's are `g`.
2. The splitter emits **every** retail data symbol global, including unambiguous file statics:

   | section | retail global | retail local |
   |---|---|---|
   | `.data` | 59 | 0 |
   | `.sdata` | 57 | 0 |
   | `.sdata2` | 57 | 0 |
   | `.rodata` | 2 | 0 |

De-`static`-ing a unit to chase this would be optimizing against a tool artifact — the data-axis
twin of the `@N`-vs-`lbl_` trap CLAUDE.md already warns about. It also makes the source *less*
plausible. Don't.

---

## 3. Taxonomy — every classified vein, and why none is workable

| class | signature | verdict |
|---|---|---|
| **Anonymous-pool pairing** (section-granular) | `missing == section size` exactly; section holds `@N` symbols | Artifact. Fixing it requires our source to *define named `.sdata2` constants* — the **banned pool-reconstruction construct**. Closed on principle. |
| **Merged-TU duplication** | equal distinct constant counts, retail merely holding more copies; our side always *smaller* | TU-boundary artifact. `objects/202` is 421→127 slots at 119 distinct on both sides. Leave it. |
| **Pool-order TU artifacts** | same value multiset, different offsets; size 4v8 / 8v4 shifts | TU-boundary artifact per CLAUDE.md. Leave the unit `NonMatching`; do not reconstruct the pool. |
| **Unowned `gap_*` bytes** | splitter-emitted `gap_NN_8XXXXXXX_section` symbols marking bytes no symbol owns | Not ours to define. Exclude from every screen — counting them manufactures a phantom gap. |
| **Genuine content** | our side missing *distinct values* the retail side has | The only real class. Exactly one instance found tree-wide (`player_SeqFn`, +8232 B, a retype — not an added constant). |

Of 25 non-`auto_` data-losing units, **13 have `missing == .sdata2` size exactly**, every one with an
overwhelmingly anonymous pool: `intersect_render` 236/236 (13 named / 40 anon), `engine/5` 176/176
(0/41), `DIMSnowHorn` 144/144 (2/33), `BossDrakor` 120/120 (0/27), `DR_LaserCan` 100/100,
`engine/19` 96/96, `CFGuardian` 88/88, `SH_thorntai` 80/80, `DFropenode` 76/76, `engine/69` and
`DIM_BossTon` 64/64, `engine/24` and `sincosf` 32/32.

---

## 4. Two standing stop-rules

### `.sbss2` is unpartitioned by construction

**`.sbss2` has ZERO spans in `config/GSAE01/splits.txt` project-wide.** The splitter partitions
`.init .text .ctors .dtors .rodata .data .bss .sdata .sbss .sdata2 extab extabindex` — `.sbss2` is
not in the list.

Consequences, both absolute:
- **No TU can own or emit any `.sbss2` byte.** Every `.sbss2` symbol is a cross-TU extern *by
  construction*, never a "definitional gap in this unit". Defining one emits data the carved
  retail object does not contain.
- **`.sbss2` is not in the `total_data` denominator**, so it can never move a score either way.

Worked case: `lbl_803E8450`/`lbl_803E8454` in `intersect_render` are dangling externs into `.sbss2`,
read once each. Retail genuinely emits an **sda21 load of a zero word**, so a plain literal cannot
reproduce them — they are *not* the banned-`lbl_`-const purge shape. They are real zero-valued const
objects that no TU is allowed to define. Leave them; do not value-name them (`gZero…` names what a
value *is*, not what it *does*).

### The `284` precedent — if the DOL moves, stop

Symbol re-partitioning that leaves the DOL byte-identical is safe bookkeeping (`object.c`). Changes
that genuinely re-lay pool words change the DOL (`284`). **DOL SHA-256 identity is the gate**;
`matched_data` is only a hint. A data "win" that moves the DOL is a regression wearing a score.

---

## 5. The screens, in run order

Run these before touching anything. The order is load-bearing — it was established by getting it
wrong (checking size before duplicate inflation misclassified `objects/202` and contradicted a
confirmed peer result).

1. **`missing == section size`?** If the shortfall equals a whole section's size exactly, it is the
   section-granular pairing artifact. Stop — nothing below will change the verdict.
2. **Distinct-values screen**, with precedence
   **missing distinct values → duplicate inflation → size → order.** A merged TU is *always* smaller
   on our side, so size alone cannot distinguish it from a content defect. Trust `.sdata2` verdicts
   above `.data`/`.sdata` ones: those carry relocation targets, so a "missing value" there may be an
   address word rather than a constant.
3. **Ownership** — does the address fall inside this unit's span in `splits.txt`? Check the *section*
   is partitioned at all before asking which unit owns it (see `.sbss2` above). Exclude `gap_*`.
4. **DOL identity** — `shasum -a 256 build/GSAE01/main.dol` before and after. Plus per-function
   `tools/unitfuzzy.py --all` equality on the whole radius for `NonMatching` units, where unit-level
   metrics and DOL identity cannot see a per-function regression.

### Gate traps that fired on this axis

- **`report.json` is not rebuilt by the default ninja target.** Build it explicitly
  (`ninja build/GSAE01/report.json`) before reading a score, and compare its mtime against the object
  you just rebuilt. A stale report reads as a clean null.
- **The retail object re-carves from `symbols.txt`.** After a rename, confirm the new name is
  actually present in `build/GSAE01/obj/...` before concluding anything from a pairing measurement.
- **`tools/autogen_data_triage.py` regenerates tracked files** (`docs/orig/autogen_data_triage.{md,csv}`)
  despite a read-only docstring. Check `git status` after running it.

---

## Attribution

- Section-granularity law, `intersect_render` decomposition, both refutations, the `.sbss2`
  construction fact, the `engine/5` null, the 25-unit census and the screen precedence fix:
  measured in this lane.
- `objects/202` merged-TU confirmation and the `engine/0` constant audit: Lane B.
- Peer commits: `d440ea21bc` (independently landed the same 12 `intersect_render` `.sdata2` names —
  worktree edits converged byte-identically, nothing to commit), `d40663374d` and `b5d4da6c62`
  (this lane's `intersect` naming batches).

## See also

- `CLAUDE.md` — the banned pool-reconstruction construct, and why it keeps being re-introduced.
- `docs/source_shape_levers.md` — the code-shape axis; its data section points here.
- `docs/rename_safety.md` — the rename gate and the stale-object race.
- `docs/splits.md` — how the splitter carves retail objects from `symbols.txt`.
