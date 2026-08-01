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
     **But `mw_version` identifies a candidate lever, it does not fire one.** The `extsb`/`extsh`
     census came back **zero surplus in every function** on `WORLDplanet.c`, and Lane B measured
     `SB_Galleon.c` — also GC/1.3 — at `extsb` **29/29**. Read the version to know *which*
     levers are worth censusing, then census before writing anything.
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

**Nor when the constant is loop-invariant rather than reused across statements.** In
`errorThreadFunc` retail materialises a hoisted `0xc080` fill *before* computing the loop bound;
naming it (`u16 fill = 0xc080;` assigned ahead of `rows`, with the loop body storing `fill`) was
**exactly inert** — LICM hoists the literal either way, so there was no re-materialisation for
the fossil to remove. The lever fires on a constant used by **two or more separate statements**,
not on one the optimiser already hoists. Swapping the two preheader statements to change the
emission order instead **regressed 99.954 -> 99.673**.

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

**Measure band width per FUNCTION, not per scope.** A narrow local trio inside a large function
is **not** the narrow-band regime, and inside one the model can invert. `player_SeqFn` (7416 B)
has an `f32 dz; f32 dy; f32 dx;` trio in a nested block, and the whole function's FP band is
only f29-f31 — it looks like a textbook 3-wide case. Retail's homes match *first-declared ->
f31* on the original `dz, dy, dx` order, yet our build only reproduced them after swapping to
`dy, dz, dx` — **the inverse of what the model predicts**. The band is shared across several
nested scopes, so it is a whole-function allocation problem wearing a narrow-band disguise.
Check the function's band width before trusting the rule, and treat a scope-local trio as
unpredictable regardless of how few values it holds.

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

### 5. The `(int)` cast / addend order — one knob, and its residual is the price

*All measurements in this section are Lane B's.*

**Shape.** A same-length transposition: retail and ours emit the same instructions in a
different order, across a pointer-vs-index computation or a commutative operand pair.

**Source.** An `(int)` cast on the index expression restores retail's evaluation order. A/B
every site — this is a live lever, not a rule you can apply blind.

**The thing that makes it different from every other lever here: it is ONE knob driving TWO
facts.** It governs evaluation order *and* commutative operand order simultaneously, and the two
can want opposite settings. When they conflict, you cannot have both, and reversing the cast to
fix a small residual transposition **costs the entire win**:

| function | with the lever | reversed to fix the transposition |
|---|---|---|
| `objRenderShadow2` | **99.956** | 99.082 |
| `renderOpMatrix` | **99.942** | 98.743 |
| `staffMtxFn_8003b620` | **97.018** | 92.289 |

**Rule that follows, and it generalises past this lever: the residual of a landed cast lever is
its price, not a defect.** When a fix leaves a small transposition behind, check whether that
transposition is the same knob's other face before treating it as new work. Three separate
functions here punish the attempt by an order of magnitude more than the residual was worth.

**Does not fire when** the two facts happen to agree — then there is nothing to trade and the
lever is simply inert.

### 6. Restore an inlined helper boundary to restore a local's lifetime

**Shape.** Retail initializes one source loop counter and derives two strength-reduced offsets
from it (`li i,0; mr heapOffset,i; mr nodeOffset,i`). A flattened reconstruction keeps the loop
in its caller and gives the counter an independent value (`li i,0; li heapOffset,0; mr
nodeOffset,i`), even though the loop body is otherwise identical.

**Source.** Recover the real `static inline` helper instead of spelling its body in the caller.
In `pathSearchBegin`, the Dinosaur Planet lineage supplied both missing boundaries:
`routeClear()` owns the 254-entry clear loop, while `routeHeapInsert()` accepts a distance and
performs the `-1 - distance` max-heap inversion internally. The SFA equivalents are
`pathSearchClear()` and `pathSearchHeapInsert()`.

**Measured.** Restoring the heap API first moved `pathSearchBegin` **97.880 -> 98.449**.
Moving the clear loop and its `i` local into `pathSearchClear()` then moved it **98.449 ->
100.000**. The unit moved **99.49424 -> 99.92317** without changing
`pathSearchAddNeighbor` (**99.780**).

**Why this is a source fossil rather than arbitrary extraction:** the donor has the same route
record, 254-entry capacity, clear loop, add-point flow, max-heap inversion, and sift-up/down
helpers. The target's two `mr` instructions are exactly the copies MWCC emits when the inlined
helper's local owns the induction lifetime.

**Do not generalize this into "factor code until registers move."** Factoring the adjacent
add-point sequence into another inline helper regressed `pathSearchAddNeighbor` **99.780 ->
99.432** by swapping its saved-register homes. Replacing that function's already-correct manual
sift-up block with the recovered helper regressed it to **99.194**. Splitting the sift-up body
where it was otherwise inert also changed the raw object identity, so it was reverted. Require
both lineage evidence and a measured gain at the exact call site.

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

**Caching a global in a local is usually right — verify before undoing it.** In the same
function, dropping the local `i` and reading `gSubtitleLineIndex` directly at each use
**regressed 96.567 -> 94.963**. Together with the pointer-IV result above, this confirmed both
reshaping choices made when that TU's boundary was recovered, rather than assuming them. When a
recent commit message records a shape decision, re-measure it against the target — but expect it
to hold.

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

**Declaration position does not move a compiler-generated temp relative to a named local.**
`objCausticReflectionRenderCb` looks like a 4-byte frame-layout bug — retail `stw r0,20(r1)` /
`addi r4,r1,20`, ours `16(r1)`. It isn't. The **frames are identical** (`-320` both); there are
*two* 4-byte temps, a `volatile float root` and the anonymous by-value `GXColor` copy for
`GXSetTevKColor`, and they are simply **swapped** between slots 16 and 20. Four spellings, all
**exactly inert** (99.991 to the digit, T=C=523): hoisting `volatile float root` out of its `if`
block — which mirrors the real `sqrtf` inline, where `volatile float y` is declared *before* the
`if` — moving it to the function's top-level declaration list, swapping two unrelated
declarations, and naming the anonymous copy. If a diff is a temp-vs-local slot swap rather than
a frame-size change, declaration position is not the knob.

**Arithmetic respelling of a constant multiply is inert.** `modelGetAmapSize` (band 2, one
instruction short) splits an `if`/`else` where retail materialises the `else` arm through
`slwi r0,r5,2; mr r31,r0` and we emit `slwi r31,r5,2` direct. `animCount << 2`, `4 * animCount`,
and a local copy of `animCount` were all **exactly inert** (97.614, 47 vs 46). Note also what
*not* to try: inverting the arms would break the branch sense retail pins with `cmpwi r4,0;
beq-` — you'd trade a match for one instruction.

**Better code is not the goal; retail's shape is.** In `hitDetect_800667ec` the source computes
`typeSlotp = slotBase + i;` and then, on the next line, `slotBase[i + 0x54]` — an obvious
missed reuse. Rewriting it as `typeSlotp[0x54]` **regressed 98.148 -> 97.399**: retail *spills*
`typeSlotp` to `368(r1)` and reloads it (4 instructions) while our version keeps it in `r14`
(2 instructions), and holding `r14` cascaded to **+18 instructions** overall. An allocator spill
decision in an 18-wide band is not source-reachable, and "our code is tighter" is a warning sign.

**Curing a defect can cost more than the defect.** Deleting four decompiler-invented pointer
locals in the same function *did* cure the `addi r0,r1,K; mr rN,r0` detour — every `addi` went
direct — but shrank the frame 656 -> 640, cascading every stack offset: **98.148 -> 97.987**.
Retail's frame is 656, so those locals are real. Check the frame size before deleting locals.

**Definition-order is the copy-class knob, and it can still go backwards.**
`modelLoadAnimations` is 12 regions of a single `r27`/`r31` swap on two copy-class parameters
(retail `id`->r31, `animBase`->r27). Since copy class is keyed on *definition* order, delaying
`buf = animBase` past the first use of `id` is the textbook probe; it **regressed
99.661 -> 99.237**.

**A score that ticks up while the instruction count walks away is the wrong direction.** An
inner copy loop reproducing retail's branch-back-to-the-count-test in `modelApplyBoneTransform`
scored **10.784 -> 10.810** — +0.026 — for **+9 instructions** (153 -> 162 against retail's 120).
Reverted. Track the count alongside the percentage.

**Free source-consistency fixes worth knowing are inert, not harmful.** `RENDER_BITS_REFILL` and
`RENDER_BITS_REFILL_NEXT` are twin macros that differ only in that one uses a two-variable form
(`addrB = bufA + curB; curB = addrB;`) and the other assigns in place. Making them consistent is
**exactly inert** — propagation folds them — so it can be done for readability at zero byte cost,
but it will not move a score.

The remaining four in this section are **Lane B's measurements**.

**The statement split does not generalise to pool-constant placement.** Splitting a statement to
move where a constant is materialised is a real lever in its own right, but it is not a
general-purpose placement knob. At `DBprotection_updateFlight` site 2 the statement was
*already* split; merging it back **regressed 99.061 -> 99.009**. Before applying it, check
whether the site is already at the setting you are about to "fix" — the same
already-at-its-optimum trap as the abs spellings above.

**A ternary consumed by an enclosing expression does not always fire.** The pattern is a real
one, but `drlasercannon_aimAtTarget` **regressed 97.660 -> 96.745** under it. Treat it as an A/B
candidate, never as a rewrite rule.

**Naming a derived induction variable creates an independent variable, not a copy.** The
intuition that a name merely labels an existing value is wrong once the value is IV-derived:
MWCC gives the named local its own web and its own update. `dll_0B_spawnEffect` **regressed
95.748 -> 95.202** and its structural count went **7 -> 8** — the name *added* a structural
difference. This is the counter-case to "give a value a name to move it": naming works on a
compiler temp with no source variable behind it, and backfires on one the compiler already owns.

**Source-level commutative operand order is inert.** Writing `a + b` versus `b + a` does not
reach the emitted operand order; MWCC canonicalises before it matters. If a commutative pair is
transposed, the knob is the cast lever above, not the source text.

## Gate reminders that cost real score today

- **A fuzzy drop can be the signature of a now-correct allocation.** `player_SeqFn` held a
  spurious 12th saved register; removing it gave retail's exact 11-wide band and took the
  function's **structural regions to 0** — but the unit read **0.029 lower**, because the *wrong*
  12-wide band happened to align more register numbers with retail than the correct 11-wide one
  does. Judge a change on **`matched_data`, `matched_code`, `matched_functions` and the
  structural-region count**, never on fuzzy alone. In that instance fuzzy fell while
  `matched_data` went 12.276% -> 100% (+8232 bytes) and a function flipped to matching.

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
- **Assert the object exists after a by-name rebuild — the path is easy to get wrong.** A batch
  helper that builds `build/GSAE01/src/main/textrender.c.o` instead of `…/textrender.o` asks ninja
  for a target that does not exist. Without an `[ -f $O ]` assertion the loop scores three **stale**
  objects and reports three false "free" verdicts; with it, the run fails loudly. Strip the `.c`
  (`${f%.c}.o`), and assert. This is the third distinct save from that one check.
- **Word-boundary every identifier check.** A bare substring guard (`"mode" in body`) fires
  falsely on `modelState`/`modelInstance`. One did, aborting a rename *after* `symbols.txt` and
  the header were already written — the partial-rename state that scores zero with a green
  build. Use `\bname\b`.

## The data axis: `matched_data` measures symbol PAIRING, not pool contents

**The axis is CLOSED and fully documented in `docs/data_axis.md` — read that before spending a
build on a data score.** The short form:

- **First screen: is `missing bytes == a whole section's size`?** If yes it is the pairing
  artifact and nothing else will change the verdict. objdiff pairs data symbols by name; our
  pool constants are anonymous (`@262`, `@263`…) while retail's are named (`lbl_803DF058`…).
- **The blindness is SECTION-granular** — a section holding *any* unpairable anonymous symbol
  scores zero as a unit, however many of its named symbols are byte-perfect. Proof:
  `intersect_render` `.rodata` (0 anonymous) 216/216 vs `.sdata2` (40 anonymous) 0/236, with
  51 of those bytes byte-identical *and* name-identical on both sides.
- **Two refuted gates, do not re-try:** naming (measured null — renaming to matching names moved
  nothing) and binding (the splitter emits every retail data symbol global, statics included, so
  retail-side linkage carries zero information; de-`static`-ing chases a tool artifact).
- **Closing the gap would require defining named `.sdata2` constants** — the banned
  pool-reconstruction construct CLAUDE.md names as the mistake that keeps recurring. Do not.
- **The distinct-values screen still answers a different, useful question.** Run its checks in the
  order **missing distinct values → duplicate inflation → size → order**: a merged TU is always
  smaller on our side, so checking size first misclassifies it.

## See also

## Two rename-gate steps added after they each caught a live error

Both fired within one naming batch and neither reached a commit. They belong in the
rename gate alongside the `--refs` radius, the by-name rebuild, the per-version check and
the `unitfuzzy` equality check (see `docs/rename_safety.md`).

**1. Name-availability grep, tree-wide, BEFORE renaming.** A name being accurate is not
enough; it must also be unused. Naming `lbl_803DB670` (`.sdata`, 1.3333334) as
`gCameraAspectRatio` collided with `camera.c`'s own live aspect ratio -- the same value at
a different address, 0x803DB268 -- and the link failed with `multiply-defined`. Run
`grep -rn '\bNEWNAME\b' src/ include/ config/` first; the correct name here was
`gStandardAspectRatio`, the standard-aspect counterpart to `widescreenAspect_803DEC1C` in
the widescreen branch that consumes it.

**2. Edit `symbols.txt` by ADDRESS, never by name or value search-and-replace.** After the
collision above, a plain `s/gCameraAspectRatio/gStandardAspectRatio/` over
`config/GSAE01/symbols.txt` renamed BOTH entries -- including `camera.c`'s symbol at
0x803DB268, which the source still called `gCameraAspectRatio`. That is a partial rename
that desyncs source from symbols **in a unit you are not editing**, so the defining unit
still builds and the damage surfaces elsewhere. Key the replacement on the full
`NAME = .section:0xADDRESS;` string and assert it matches exactly once.

## The high-water-mark regression audit (axis CLOSED — sized, not worth working)

This repo records scores in commit messages (`fn 98.840->99.476`, `99.444->100`), which
makes history an audit corpus: any function now scoring below a score once claimed for it
is a candidate silent regression. Two peer recoveries proved the idea (`4e4e3ff587`
+0.586 on `streamsLoadedCallback`, `ca7470b8b1` +0.636 on `renderSunAndMoon`). The full
sweep was run once; the axis is closed. Do not re-run it without new evidence.

### Method
Parse `git log --all --pretty='%H%x01%s%x02%b%x03'` for `<symbol> <A>-><B>` pairs
(also `->`, `→`, `=>`); high-water for a symbol is `max(A,B)` over all claims, since a
commit documenting a regression still records the higher score. Exclude tree-wide words
(`matched_code`, `tree`, `fuzzy`, ...). Join against current per-function fuzzy by
sweeping `tools/unitfuzzy.py` over every unit (~0.06 s each, ~1 min for 927). Functions
absent from `unitfuzzy` output are at 100% and cannot be below a high-water.
Rank by `(high - current) x size` to get bytes, not percentages.

### Two preconditions, both learned by getting them wrong
1. **Run only on a freshly built tree.** `unitfuzzy` reads objects, so stale objects
   manufacture phantom regressions. The first sweep reported 51 hits including three
   `newshadows.c` functions that were already fixed in HEAD; `ninja all_source` rebuilt
   988 objects and all three returned to their high-water values. Always
   `locked_ninja` + `all_source` immediately before sweeping.
2. **Filter claim precision.** Commit messages write 2 dp, `unitfuzzy` reports 3+, so a
   claim of `99.65` against a current 99.647 looks like a 0.003 regression and is nothing.
   **19 of 48 hits were this.** Discard `delta < 0.01`.

### Verdict taxonomy
A hit is a lead, not a verdict. Classify from the claiming commit's message before
measuring anything:
- **stale-claim** — measured in a context that no longer exists (different compiler or
  file). `zlbDecompress` 53.5 vs a 76.26 claim measured when zlb was inside
  `pi_dolphin.c` under MWCC with a `#pragma optimization_level` island; zlb is now its
  own ProDG unit. That single entry was 79% of the axis's apparent bytes and is a mirage.
- **accepted-trade** — the high score was bought with a construct since purged on
  principle. Not a lost score, a paid principle. Cleanest examples: `sceneDraw`'s claim
  names its own "load-bearing volatile CSE-defeat pun"; `expgfx_updateSourceFrameFlags`
  98.87->92.98 is annotated "PRAGMA WALL PROVEN"; `expgfxGetSlot` 97.70->93.68 is
  "(goto-capped fn)". Also covers artificial TU splits merged back per CLAUDE.md
  ("accept match regressions"): `renderOpMatrix`/`modelLoadMtxsToGx` both reached 100.000
  only as a separate `.c` compiled with `-opt nopropagation` (`88607918a`), merged back by
  `13eaecb9e4`.
- **tu-context** — claimed before a TU merge/split, so the unit profile differed.
  `expgfx_updateActivePools`' 99.67 predates the 7-fragment merge; its `(u16)` cast lever
  still exists in today's source, so nothing was lost but the fragment's flag profile.
- **candidate** — none of the above; verify by measuring at the claiming commit before
  proposing a recovery.

### Sizing result (2026-08, 42,030 commits, 2,006 claimed symbols)
48 functions below high-water -> 19 rounding noise -> 29 real -> 7 lane-owned -> 22 mine.
Of those, `zlbDecompress` (stale-claim) is 534 of ~674 bytes, and the next three largest
(`expgfx` x3, 65 B) are accepted-trade or tu-context. **Addressable remainder ~75 bytes
across ~18 functions at 2-5 bytes each — below the effort line.** The table lives in the
audit commit body if anyone wants the tail.

- `docs/data_axis.md` — the data axis, closed: the section-granular pairing law, two refuted gates, the vein taxonomy and the screen order.
- `docs/band_width_worklist.md` — where a structural fix can stick (`structB` vs `regB`).
- `docs/rename_safety.md` — the rename gate and the stale-object race.
- `docs/per_tu_flag_evidence.md` — per-TU flag measurements, for whoever adjudicates them.
- `CLAUDE.md` "A few MWCC facts" — the high-frequency codegen rules this file builds on.
