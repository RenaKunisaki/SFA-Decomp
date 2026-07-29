# Source-shape levers — what to try once `structB` says a fix can stick

`docs/band_width_worklist.md` answers **where** a structural fix can stick. This file answers
**what to write** when you get there, and — more importantly — **when not to bother**.

Every lever below is recorded with the measurement that landed it *and* the measurements that
refuted it elsewhere. **A lever with known non-firing cases is worth more than a list of wins**:
the refutations are what stop the next lane spending four builds on a dead axis.

Convention used throughout: `before -> after` are `tools/unitfuzzy.py` per-function scores.
"exactly inert" means identical to the digit, which is itself evidence (the front end folded
the two spellings into one expression tree).

## The screen, before any lever

Run these in order. Each one can end the round without a build.

1. **Rank by `structB`, never by missing bytes or by how close the percentage looks.**
   Missing bytes measure how much is wrong; `structB` measures how much is *reachable*.
   The near-flip band (>=99%, <100%) is **structurally empty** — six candidates triaged, all
   six with **zero instruction-count delta**, four pure register permutation plus pool
   relocations. Functions reach 99% precisely because everything source-addressable is
   already gone.
2. **Check whether the function is already worked.** The `structB` sweep has no memory. Two
   `grep`s — the memory files and `git log` on the file — distinguish "largest unworked entry"
   from "documented cap". `worldplanet_update` was briefed as the former and was the latter: a
   120-permutation declaration sweep had already run on it, flat, recorded as "organic residue".
3. **Read the unit's `mw_version` and cflags — and note whether the informative flag is
   *present* or *absent*.** This is asymmetric and the asymmetry matters:
   - flag **present** (e.g. `engine/86` builds `noschedule`) → a load-*position* difference
     **cannot** be the scheduler, so it is promoted to source-reachable. This is what cracked
     `CameraModeArwing_update`.
   - flag **absent** (e.g. `main/render.c`, plain `cflags_base`, scheduler on) → the step says
     **nothing**. Absence is a stop sign, not a green light. Step 3 can promote a difference to
     source-reachable; it can never demote one.
   - `mw_version` is part of this read. `WORLDplanet.c` is **GC/1.3**, a third compiler
     (main lib 2.0, audio/MSL 1.2.5n). The GC/2.0 lever catalogue may not transfer, and
     compiler-specific levers become worth censusing — GC/1.3 reads plain `char` unsigned, so
     retail `extsb` needs an explicit `s8`.
4. **Look for a sibling-site control inside the same function.** If the same construct gets
   two different treatments in one basic block, the difference is operand-dependent and the
   operand is the knob. In `modelRenderInterpolateRootTransform` a *full* 64-bit AND and a
   *low-half-only* AND sit in the same block — which is what identified operand **type** as
   the lever there.

## Levers that fired

### 1. The `local = CONST` fossil

**Shape.** Retail holds a constant live in a saved register and reuses it; we re-materialise
it at each use.

```
retail: li r31,0 ; stw r31,0(0) /*global*/ ... stw r31,0(r30) /**slot*/
ours:   li r0,0  ; stw r0,0(0)             ... li r0,0 ; stw r0,0(r30)
```

**Source.** Recover the fossil assignment that made the constant a named local:

```c
gSubtitleActive = none = 0;     /* not: gSubtitleActive = 0; */
...
*slot = (void*)none;            /* not: *slot = NULL; */
```

**Measured.** `subtitleStop` **90.522 -> 97.174**, diff regions 10 -> 6.

**Why it is a real fossil and not a lucky spelling:** it also produces retail's *fourth* saved
register (`stw r28,16(r1)` / `lwz r28,16(r1)`) that we were missing entirely. A spelling that
only moved one instruction would not change the prologue.

**Generalises to:** any site where retail keeps one live constant and we re-materialise. This
is the integer sibling of the already-recorded `(v = 0.0f)` in-place float idiom.

**Does not fire when** the local is only read once — MWCC propagates it away. `GXColor kc =
temp;` before a by-value call, and `int amapBytes = animCount * 4; size = amapBytes;` were both
**exactly inert**.

### 2. Uniform declaration-order band model (narrow width)

**Claim.** At narrow band width, try the **uniform** model first: treat *all* saved-band values
as one declaration-keyed population filling `r31` downward. Do this before reaching for
CLAUDE.md's split copy-class/load-class rule.

**Evidence.** `subtitleStop` has four band members. `oldDelay` is a call return — copy class —
and the stated rule puts copy class in the **top** `|C|` registers. Retail puts it at **r28, the
bottom**. The uniform model predicted retail's homes exactly (`none`=r31, `slot`=r30, `i`=r29,
`oldDelay`=r28, i.e. declaration order `none, slot, i, oldDelay`) *and* predicted our own
pre-fix layout exactly from our own declaration order. **Two independent correct predictions.**

**Measured.** Reordering the declarations to match: `subtitleStop` **97.174 -> 97.717**,
regions 6 -> 3.

**Does not fire when** the band is wide. See the cliff in CLAUDE.md; at band >=5 declaration
sweeps are provably flat, and "narrow band" means the assignment is **predictable, not
steerable** — a 120-permutation sweep on `worldplanet_update` (recorded) and a 120-permutation
sweep on `playerUpdate` both returned zero movement.

### 3. Store width — `u16*` drops a redundant `extsh`

**Shape.** `*(s16*)dst = <int expr>;` makes MWCC sign-extend before a `sth` that truncates
anyway.

```
retail:  add r10,r10,r11 ;              sth r10,0(r4)
ours:    add r0,r4,r0    ; extsh r0,r0 ; sth r0,0(r23)
```

**Source.** Store through `u16*` instead of `s16*`. Semantically identical — `sth` keeps the
low 16 bits either way.

**Measured.** `modelApplyBoneTransform` **3.172 -> 10.784** across ten sites, `extsh` count
6 -> 0.

**The tell that it was worth doing:** the score more than tripled on a **net +1 instruction**.
That means the win was *re-alignment*, not deletion — removing the interleaved `extsh` let the
surrounding stream line up. Expect this signature; a lever that only deletes instructions
usually moves the number much less.

**Sweep it, don't chip at it.** After landing one site, census the whole unit rather than
hunting site by site:

```
objdump -M gekko -drz on both objects; count extsh/extsb per function; list only functions
where target and ours disagree.
```

On `model.c` that retired the axis in one command — the only surplus left was six `extsb`
inside two paired-single-walled functions.

### 4. Operand signedness widens a 64-bit expansion

**Shape.** MWCC folds the high half of a 64-bit operation away when it can prove it is zero;
retail keeps both halves live.

**Source.** Widen/sign the operand that feeds the expansion. `u64 h = render_readPackedU16(tp);`
-> `s64 h`. Semantically identical (the callee returns `u16`, so 0..65535 either way).

**Measured.** `modelRenderInterpolateRootTransform` **95.774 -> 95.846**, regions 82 -> 80.

**Isolation matters.** `s64 masked` alone reached the *same* score; `s64 nib` alone was inert
and **cancelled the gain** when combined. Probe operands one at a time.

**Does not fire on** neighbouring operands: `s64 bitpos` (plus dropping its three `(s64)` casts)
**regressed 95.846 -> 95.521**; `s64 vA`, `s64 maskConst` and `int flags` were **exactly inert**.
The axis is real but narrow — one operand carried it.

**Watch for the reframe this needs.** The `and`/`xor` pairs that look like source-level masking
in this function are **MWCC's 64-bit variable-shift expansion**, not source ANDs. Several probes
that added mask variables failed because they targeted a construct that does not exist in the
source. Read the expansion before writing the fix.

## Refutations worth knowing before you spend a build

**Transcribing the target's instruction order into C is not recovering its source.** Retail's
blend in `modelApplyBoneTransform` is operation-major (three `mullw`, three more, three `add`,
three `srwi`...). Written literally in C it forces all six components live simultaneously and
MWCC spills the function apart: **0.000%**, 192 instructions against retail's 120. Retail can
hold six live only because they sit in dedicated registers.

**"Indexed, not walked" has a documented counter-site.** The law says respell a source cursor as
in-loop indexing so strength reduction owns the IV. It went the **wrong way twice**:
`subtitleStop` **97.717 -> 71.935** (retail genuinely wants the source-level pointer IV), and
`worldplanet_update` **99.031 -> 98.871** with `tbl->flightPathObjectIds[i]` replacing pointer
punning. At the latter site the residual is the **base** being held rather than the index, so no
indexing spelling reaches it. Check which of base/index retail keeps before applying the law.

**Address *shape* is not a knob; address *position* is.** `we + 1` vs `&we[1]`, and `&vb[0]` vs
`vb`, are **exactly inert**. But naming an address hoists its computation: `tp =
gSubtitleLineTimes + i;` before an `if` moved the work above the short-circuit branch and cost
**96.567 -> 90.634**.

**Condition spelling is not a knob either.** `curTime >= (&gSubtitleLineTimes[i])[1]` and
splitting `&&` into nested `if`s were both **exactly inert** — MWCC folds them to one expression
DAG. If two spellings tie to the digit, stop probing that expression.

**"Give the temp a name to move it" is usually propagated away.** It is the documented counter
to the single-use `r0` sink, but a named copy before a by-value call, and a named single-use
temp in `modelGetAmapSize`, were both inert. Before spending a build on it, check the target
asm: in `objCausticReflectionRenderCb` retail **re-materialises the array base separately at
each site** (two independent `lis`/`addi` pairs), which rules out a shared base local *without
compiling anything*.

**Check whether a residual is the cost of a lever already at its optimum.** Four abs spellings
on `trackGetNearestGroundOffset` all scored **worse** than the code already in the tree
(98.231 baseline vs 96.846 / 96.769 / 94.462). The existing form was already the best of the
family.

**Some walls are policy, not compiler.** `modelBoneTransforms_next` has **no prologue or
epilogue**, takes its cursor in `r20` and mutates it, returns three values in `r10`/`r12`/`r15`,
and clobbers callee-saved `r21`/`r22` without saving either. No compiled C does that under the
EABI. It is hand-written assembly; inline asm is banned outside `src/dolphin/`, and the unit
stays `NonMatching`. Do not reach for global register variables — they would not reproduce it
anyway (MWCC still emits saves for any callee-saved register it allocates, and three register
return values are not expressible).

## Gate reminders that cost real score today

- **A per-function `fn_flag_probe` MATCH is not a unit-level win.** The probe scores one
  function and is blind to what the flag does to the rest of the TU. On `subtitle.c` the
  profile that made `subtitleUpdateAndDraw` MATCH collapsed `subtitleBuildLineTable`
  100 -> 19.887 and the unit 98.747 -> 57.135. Read `PROFILES` in the tool before translating
  a verdict into a `configure.py` edit — the profile *names* do not describe their flag sets.
- **DLL objects need a by-name rebuild after any rename.** Nothing in the default target graph
  builds them. A stale one is invisible: `dlls/modgfx/90` was **unscorable** (`no
  fuzzy_match_percent for this unit`) with a green build, and a touch-plus-rebuild restored it
  to 99.21875. Note that an unscorable unit is **excluded from the aggregate, not counted as
  zero**, so this repair *lowers* reported fuzzy — it is still correct.
- **Word-boundary every identifier check.** A bare substring guard (`"mode" in body`) fires
  falsely on `modelState`/`modelInstance`. One did, aborting a rename *after* `symbols.txt` and
  the header were already written — the partial-rename state that scores zero with a green
  build. Use `\bname\b`.

## See also

- `docs/band_width_worklist.md` — where a structural fix can stick (`structB` vs `regB`).
- `docs/rename_safety.md` — the rename gate and the stale-object race.
- `docs/per_tu_flag_evidence.md` — per-TU flag measurements, for whoever adjudicates them.
- `CLAUDE.md` "A few MWCC facts" — the high-frequency codegen rules this file builds on.
