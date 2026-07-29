# Per-TU compiler-flag evidence

**This is an evidence file, not a proposal.** Nothing here recommends a `configure.py` change.
Per `CLAUDE.md`, pragmas may only be configured at the TU level via `configure.py` cflags, a
DOL-confirmed TU must never be split for match percentage, and only DOL section / pool /
function-order / source-tag evidence may redraw a boundary. Every row below is recorded so that
whoever holds that adjudication has the measurements in one place instead of scattered across
three lanes' notes.

## Why this file exists

Working the narrow-band frontier (`tools/bandscreen.py --max-band 3 --struct-only`, the regime
where the register-band assignment is predictable and structure is the only free variable),
**four of five `regB == 0` chases resolved to compiler configuration rather than source shape.**
That is a different kind of residual from the ones the project normally fixes, and it is not
actionable from a lane. The pattern is worth adjudicating as a whole rather than one function at
a time.

## The findings

`Δfn` / `Δunit` are `fuzzy_match_percent`. "Collateral" is the point of the table: in every case
where a profile *does* reproduce retail, it is unlandable because of what it does to the rest of
the unit.

| # | unit | function | profile that matches / would match | Δfn | Δunit | collateral — why it is unlandable |
|---|---|---|---|---|---|---|
| 1 | `dlls/objects/195_Player/player.c` | `playerUpdate` (2372 B) | **`-opt nocse`** (also `nocse,nopropagation`) — **byte-exact** | 98.432 → 100 | not measured | TU-wide flag change; the unit has 22 sub-100 functions, none re-measured under `nocse`. Conflicts with row 2 — see *The player.c conflict* below. |
| 2 | `dlls/objects/195_Player/player.c` | `fn_802AABE4` (352 B) | **`-opt nopropagation`** (also `nocse,noprop` / `noprop,noauto` / `noprop,inloff`) — **byte-exact** | 97.045 → 100 | not measured | Same TU as row 1 but a *different* profile. Both cannot hold. |
| 3 | `main/audio.c` | `streamsLoadedCallback` (280 B) | **none identified** | 97.643 → 97.057 with the correct source (see note) | 99.93366 → 99.91717 | No `-opt` knob reproduces it. The TU is already `nopeephole,noschedule,nostrength`, so it is outside the peephole/CSE/propagation classes. **The most interesting row: the residual class is not fully covered by the flags anyone has enumerated.** |
| 4 | `main/gameloop_buttonobj.c` | `removeButtonObject` (220 B) | **none** — hypothesis *unconfirmed* | — | — | Retail emits `srwi r0,r3,3; cmplwi r0,0`; we fuse to `srwi. r0,r3,3`. The TU is `-opt noschedule` only (peephole **ON**) while the rest of `main/` is `nopeephole`, so peephole fusion was the natural hypothesis — but `fn_flag_probe.py`, **all of whose profiles are `nopeephole`**, reports no match. Peephole-off alone does not fix it. Recorded as unexplained, not as a peephole finding. |
| 5 | `dlls/.../SaveGame` | `SaveGame_gplaySetObjGroupStatus` | `-opt peephole,noschedule` | 97.981 → 87.746 | 99.743 → 92.652 | Catastrophic: the profile that helps the region destroys the unit. A clear negative. |
| 6 | `main/gametext_tail.c` | `textMeasureFn_80016c9c` (1836 B) | `-opt noloopinvariants` | 98.039 → 98.943 | not recorded | Breaks `gameTextRenderById` **100 → 95.404**. Net loss; the classic shape of this whole table. |

Rows 1–4 were measured in this lane. Rows 5–6 were measured elsewhere and are relayed here with
their reported numbers so the set is in one place; re-measure before relying on them. A further
`acosf` / `sincosf` / `gameloop_buttonobj` profile sweep exists in the same lane's records and is
**not** reproduced here because this file should not carry numbers nobody in it measured — pull it
from that lane before adjudicating.

### Note on row 3

`streamsLoadedCallback` is the one row where the *source* question is settled and only the flag
question is open. Line 483 read `gAudioPendingLoadFlags &= ~AUDIO_LOAD_STREAMS;` while **7 of the 8
sibling sites in the same file** write `~(u64)AUDIO_LOAD_X`. The `(u64)` width is load-bearing (cf.
`docs/STYLE.md`): it forces the mask to be materialised — retail `li r0,-5; and r0,r3,r0` — instead
of folding to our `rlwinm r0,r0,0,30,28`. Adding the cast takes the function from 87-vs-86
instructions / 39 positional diffs to **87/87 with REGBLIND 2**, i.e. one moved instruction from
byte-exact.

That one instruction is `li r4,0` — **the loop-invariant stored value** of `s->flag = 0`, not an
induction variable (the counter is strength-reduced away; the body is `stb r4,21(r5)` ×8 with
`addi r5,r5,176`). Retail materialises it in the loop preheader; we hoist it above the two flag
read-modify-writes. Eight source spellings are exactly inert against it — `i = 0; for (; …)`, the
`while` form, an inner block scope for `i`, `i` declared last, `i = i + 1`, a named `int zero`
assigned immediately before the loop, a `u8 zero = 0` in an inner block, and moving the flag writes
below the data reads (that one is *worse*, 99.835). It is an LICM constant-placement decision.

**The cast is the correct source and is deliberately not committed**, because it measures −0.016 on
the unit. It should be landed together with whatever fixes the `li` placement, not before.
| 7 | `dlls/objects/195_Player/player.c` | `playerStateTryCastSpell` (964 B) | **none identified** — not source- and not flag-reachable | — | — | Retail hoists the shared `lis r6,32` high half of two identical `0x200001` call arguments into a saved reg (`lis r30,32`) and derives `addi r6,r30,1` at each site; we rematerialize `lis;addi` twice. In an isolated 2-call probe **4 source spellings** (bare literal, `int f = 0x200001`, `b+1`, `b|1`) and **7 `-opt` profiles x 4 `-O` levels** all emit the un-hoisted form, so it is neither a literal spelling nor an enumerated flag. Most likely a consequence of whole-function register pressure — i.e. band-adjacent, not a source lever. **Third data point for the player.c conflict: this is a *more*-CSE behaviour, while row 1 wants `-opt nocse`.** |

## The `player.c` conflict — an open TU-boundary question

*(Updated: rows 1, 2 and 7 are now three mutually incompatible requirements in one TU —
`nocse`, `nopropagation`, and a hoist that needs* more *CSE than we currently get. Three
conflicting profiles is stronger evidence for a merged-TU boundary than two.)*

`playerUpdate` is byte-exact under `-opt nocse`; `fn_802AABE4` is byte-exact under
`-opt nopropagation`. **Both are in `src/dlls/objects/195_Player/player.c`, and no single profile
satisfies both.**

`CLAUDE.md` names this exact situation: *"conflicting profiles may show that the current unit merges
multiple real TUs, but only DOL evidence may justify correcting that boundary. Never split a
DOL-confirmed TU to isolate a flag profile."*

So this is recorded as a question, with two explicit caveats:

1. **No DOL evidence has been gathered.** Nobody has checked the `.text` function order, the
   `.sdata2` pool partition, `.data`/`.bss` ownership, or source tags around these two functions.
   Until that is done, "merged TU" is a hypothesis with one supporting symptom, not a finding.
2. **A conflicting profile is not sufficient on its own.** Eight other `player.c` functions were
   probed across all nine profiles and matched none of them, so most of the unit's residual is not
   flag-shaped at all. The conflict may equally be two unrelated coincidences.

The useful next step is the DOL audit, not a split.

## How to reproduce

Two tools, already in the tree:

- **`python3 tools/fn_flag_probe.py <unit.c> --symbol <fn> [--symbol …]`** — per *function*, tries
  nine profiles and reports which (if any) make that function **byte-exact**. Fast; the right first
  screen. Its matrix is `as-configured / prop / noprop / nocse / nocse+noprop / noprop+noauto /
  prop+noauto / noprop+inloff / prop+inloff`. **Every one of those profiles is `nopeephole`**, so a
  peephole-*on* hypothesis is not testable with this tool — that is why row 4 is unresolved.
- **`python3 tools/cflag_sweep.py <object_key> <unit_suffix>`** — per *unit*, sweeps TU-level cflag
  profiles and gates on true objdiff fuzzy. Restores and re-runs `configure.py` on exit, including
  on error. This is the tool that produces the collateral column.

**Gating rule.** A per-function byte-exact match is *not* a result on its own. The flag is TU-level,
so the only number that decides anything is the **whole unit's** `fuzzy_match_percent` under the
candidate profile, measured with `tools/unitfuzzy.py` (or `report.json`) — never a diff count, and
never the target function alone. Rows 5 and 6 exist precisely because the per-function number and
the per-unit number pointed in opposite directions.

## Standing caveats

- Do not change `configure.py` from a lane.
- A row that names a profile is *not* a recommendation to adopt it; every such row in this table is
  a net loss at unit scope, which is the finding.
- Re-measure before acting: these numbers were taken against a moving tree.
