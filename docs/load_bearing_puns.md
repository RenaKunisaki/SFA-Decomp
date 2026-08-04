# Load-bearing puns — do-not-fix registry

A **raw pun** is a byte-offset dereference (`*(T*)(base + K)`, `(T*)(base + K)`,
an `int` walked with byte arithmetic) where a named struct field exists and the
obvious member spelling would be more readable.

Most of them are free: rewriting the offset as the field is byte-identical, and
those are not listed here — they get rewritten. This file records the ones that
are **not** free. A pun is **load-bearing** when the member spelling gives the
front end something extra to fold or common-subexpression, and retail's code
proves it did not fold. Rewriting one costs bytes, so it must not be "cleaned".

## How to read an entry

Every row carries the measurement that made the verdict, taken with the unit
rebuilt against a pristine control at the same parent and compared with
`tools/obj_equal.py` — never with the score alone.

**A score gate cannot see this class.** `454_DIMCannon`'s `modelRotation[1]`
below loses 8 bytes at an *identical* fuzzy percentage. Fuzzy is a summary, not
a comparator: gate on `obj_equal`.

**Scope is the function, not the file, and not even the spelling.** In
`tricky.c` the same `state + 0x7XX` base is load-bearing for the child-object
pointer slots and free for the float and bitfield slots *in the same function*.
Measure the site, not the pattern.

## The law that triages the rest

**FIXED vs INDEXED.** A fixed-offset re-spelling is usually free; one whose
result is then subscripted by a **variable** usually is not — the front end
folds `b + K` into the scaled index of `((S*)b)->arr[i]` while retail keeps the
two-step. A **constant** subscript is not a hazard: `((GameObject*)cursor)->childObjs[0]`
on a walking `cursor` is byte-identical (`lightmap.c`, measured).

---

## Registry

### modgfx `packet.commandCount` — ten units

`(u8*)`-cast pointer difference divided by the command stride. The cast is
precisely what stops MWCC folding the pointer difference; retail **emits the
divide-by-24**. `20`, `&commands[20] - commands` and `(commands + 20) - commands`
all fold and cost **32 bytes** each.

Units: modgfx 124, 125, 126, 127, 129, 130, 131, 132, 133, 134.

### `196_Tricky/tricky.c` — `trickySelectQueuedCommandTarget`

The scan walks the queued-command records as an `int` incremented by 8 with
fixed byte offsets `+0x748` / `+0x74c` / `+0x74d`.

| spelling | fuzzy | size |
| --- | --- | --- |
| retail: `int ref = (int)state; ref += 8` | **100.0** | 316 |
| `TrickyCommand* cmd; cmd++` | 99.177216 | 316 |
| `state->commands[i]` | 97.974686 | 316 |

Both alternatives are the same length as retail, so the residual is pure
colouring — the walking `int` is what pins it.

### `196_Tricky/tricky.c` — `Tricky_updateSideCommandPrompts`, child pointers only

`*(u32*)(state + 0x7a8)` / `*(state + 0x7b0)` (write) and
`(GameObject**)(state + 0x7a8)` / `(+ 0x7b0)` (address-of), 4 sites:
100.0 → **99.0**.

The same function's `*(float*)(state + 0x7ac)` / `(+ 0x7b4)`, its
`((TrickyPackedSlots*)(state + 0x7bc))->` accesses and its
`*(u8*)(state + 0x7bc) >> n & 3` bitfield reads are **free** and have been
rewritten. Only the child-object pointer slots are load-bearing.

Two sites of the *same spelling* in `Tricky_free` (`(GameObject**)(state + 0x7a8)`
and `(+ 0x7b0)`) are **free** — they were rewritten. The source previously
carried a blanket `/* raw: arrow form shifts bytes */` comment on all six; that
comment over-claimed, and the measurement above is what replaced it.

### `main/objprint_dolphin.c` — `objFuzzRenderCb`

`*(void**)(rop + 0x38) != NULL` — a pointer-width null test on
`Shader.indTextureId` (an `s32`), 2 sites. Spelling it
`((Shader*)rop)->indTextureId != 0`: 99.82734 → **99.65468** (size 2780
unchanged).

### `523_FireFly/FireFly.c` — `firefly_activeTick`

`(f32*)(player + 0x18)` off an `int player`, 2 sites. As
`&((GameObject*)player)->anim.worldPosX`: 100.0 → **98.63603** (size 1088).

### `529/529.c` — `wmwallcrawler_update`

`(f32*)(ob + 0x24)` off a `u32 ob`. As `&((GameObject*)ob)->anim.velocityX`:
100.0 → **99.31814** (size 3860).

### `625/625.c` — `drakorhoverpad_updateMain`

`p + 4` is `DrakorHoverpadState.curve`, and the *same statement* already spells
the sibling argument `&((DrakorHoverpadState*)p)->curve` — so the pun is
deliberate, not an oversight.

| spelling | fuzzy |
| --- | --- |
| retail | **99.843346** |
| `(Curve*)&(...)->curve` (2 sites) | 99.386420 |
| both casts rewritten (4 sites) | 99.073105 |

### `dlls/engine/2/2.c` — `ObjSeq_start`, the cross-symbol reach

`base + 0x2bd4 / 0x2bd8 / 0x2bdc` where `base = gObjSeqRuntimeBuffer` lands
inside a **different `.bss` object**: `objSeqOverridePos[3]`, which
`ObjSeq_setOverridePos` writes through its own symbol. Reading it back by that
same symbol — the spelling the writer uses — is **not** free: 99.311295 →
**99.011020** (size 2904 unchanged, `matched_code` unchanged, 4 extra
instructions). Retail reaches the neighbour through the first object's symbol,
so the relocation target *name* is load-bearing here even though the address is
the same.

The whole `base + 0x3XXX` cluster in this unit is the same thing: `SeqRunTables`
and `ObjSeqRunBgState` are two composite overlays over a **run of adjacent .bss
arrays**, not one struct. That makes it the `.bss`-order lane's question (A74),
not a field-naming one.

### `454_DIMCannon/DIMCannon.c` — `modelRotation[1]`, 5 sites

**Loses 8 bytes at an identical score (99.967674 both ways.)** The only entry
here that no score gate can see. This is why `obj_equal` is the gate.

### Carried forward, measured by earlier lanes

| site | effect |
| --- | --- |
| `600_DR_CloudRun` `&base->speedMax[idx >> 1]` | −4 bytes |
| `201_Baddie` `&obj->anim.velocityX` (3 spellings) | +12 bytes |
| `598_DIMSnowHorn` `data + 0x980` (3 spellings) | +4 bytes |
| `601_SB_Cloudrun` `rotZ` / `rotY` | −4 bytes each |
| `main/objhits.c` `u32 ob = (u32)objB;` in `ObjHits_CheckSkeletonPair` | 99.734184 → 99.68724 (a B30/B31 demote device) |
| `226/226.c` `staff_setupSwipe` param re-type | 99.91625 → 99.74876 |
| `376_DFSH_Shrine` `dfshShrine_updateHoverMotion` param re-type | 100.0 → 99.7479 |

## Declined for want of evidence, not for bytes

These were never measured because there is nothing to name them from. They are
not load-bearing; they are simply unnamed.

- `dlls/engine/2/maketex.c` `gSaveCardImageBuffer + 0x20 / 0x2a40`,
  `gSaveCardIoBuffer + 0xa40` — byte offsets into a serialized memory-card
  image. The offsets *are* the file format; no struct exists to name them with.
- `dlls/engine/11/11.c` the `dstv` vertex record — no named type exists, and
  naming it would be an invention.
- `main/lightmap.c` `q + 0x3f48` / `+ 0x4108` / `+ 0x4164` where
  `q = (char*)gLightmapDrawQueue` and `gLightmapDrawQueue` is declared
  `char[0x3F48]` — these address *past the end* of the array, i.e. the
  neighbouring `.bss` objects. That is the `.bss`-order lane's territory
  (A74), not a field-naming question.
