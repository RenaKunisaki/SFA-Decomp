# Renaming a function safely

A pure rename can silently destroy a unit's score with a completely green build.
Five game-text renames once produced `locked_ninja.sh` EXIT=0, 0 FAILED, and:

```
textrender_run.c     99.40815 -> 96.46698
textrender_drawbox.c 99.95742 -> 93.76387
gametext_tail.c      99.06977 -> 55.46511
```

## The mechanism

1. **`config/GSAE01/symbols.txt` is build input.** `configure.py` lists it in
   `config.split_deps`, and the split step is
   `build/tools/dtk dol split config/GSAE01/config.yml build/GSAE01`.
   Editing it re-runs the splitter, which **re-carves the whole retail tree
   under `build/GSAE01/obj/`** with the new names. (CLAUDE.md's "retail objs are
   READ-ONLY" is about *manual* interference — the splitter owns them.)
   Note the carve is a *side effect*: ninja's only declared output for that rule
   is `config.json`, so the `.o` files are untracked and there is no dependency
   edge that would catch an inconsistency.

2. **objdiff pairs functions by symbol name.** If our object and the retail
   carve disagree, the pair is lost. A **retail-only** symbol scores **zero**;
   an **ours-only** symbol is simply *not scored* (harmless -- that is what a
   `static` helper retail inlined looks like, and 65 units are in that state
   today, all benign).

3. **objdiff also compares relocation target names.** So a name mismatch is not
   confined to the defining unit: it shows as a diff at **every call site in
   every calling unit**. That is what turns a one-function slip into a
   three-unit collapse.

So the failure is a *partial* rename -- `symbols.txt` and the C source
disagreeing, or agreeing in the defining unit while some other unit still
carries the old name. The build stays green because nothing about *linking* is
wrong.

## Why two functions were safe and five were not

Blast radius, and it is measurable:

```
$ python3 tools/pairing_check.py --refs gameTextLoadCompleteCallback
'gameTextLoadCompleteCallback' is referenced by 1 unit(s):
      6 reloc(s)  main/textrender_run.c

$ python3 tools/pairing_check.py --refs gameTextRun
'gameTextRun' is referenced by 10 unit(s):
     12 reloc(s)  main/pi_dolphin.c
      3 reloc(s)  main/gameloop_main.c
      ... 8 more
```

The safe commit renamed two leaf callbacks with a **one-unit** radius. The
failing one included a widely-declared API function with a **ten-unit** radius,
and only the edited unit was checked.

## The gate

Renames are **not** byte-neutral and `tools/byteneutral.py` cannot gate them
(the md5 must change). Before committing any rename:

1. `python3 tools/pairing_check.py --refs <oldname>` — get the blast radius.
2. Rename in **both** `config/GSAE01/symbols.txt` and every C occurrence.
3. `tools/locked_ninja.sh` as its **own step**; read EXIT before continuing.
   ⚠️ Then rebuild **every touched object by name** —
   `tools/locked_ninja.sh build/GSAE01/src/<...>.o`. Because the re-carve is an
   undeclared side effect (see above), **ninja has no dependency edge telling it
   our objects are stale relative to the retail side**, so it will happily leave
   them alone: the splitter re-carves retail with the new name while your own
   object still holds the old one. That is a *complete, correct* rename
   measuring as a total collapse.

   This is NOT only a DLL problem — DLL objects are additionally missing from
   the default target graph, but the staleness bites anywhere. Observed live on
   `main/shader.c` after someone else's rename: `pairing_check.py` reported four
   retail-only symbols and the unit read **90.49737**; a single
   `locked_ninja.sh build/GSAE01/src/main/shader.o` restored it to **99.43728**
   and the tree-wide count returned to the expected 2. Nothing was ever wrong
   with that rename.

   Measured four ways on `linkDrawFn_801302c0` in `dlls/engine/60`:

   | state | unitfuzzy |
   |---|---|
   | clean baseline | 99.88345 |
   | C source only, `.o` rebuilt by name | 91.72495 |
   | `symbols.txt` only | 91.72495 |
   | **both renamed, bare `locked_ninja.sh`** | **91.72495** ← stale `.o`, artifact |
   | both renamed, every `.o` rebuilt by name | 99.88345 |

   Only the last row is the truth. Do not revert on the fourth row.
4. `python3 tools/pairing_check.py` — must report **0 retail-only** symbols.
   (Two are expected and permanent: `gap_03_80006C6C_text` in `main/render.c`
   and `gap_03_8028C964_text` in `targimpl.c` — foreign-toolchain blobs.)
5. `python3 tools/unitfuzzy.py <unit>` on **every unit from step 1**, not just
   the one you edited.

## Safe subset

Lowest risk to highest:

- **`static` function with no `symbols.txt` entry** — our object only; the
  retail carve never named it. Renaming cannot unpair anything.
- **Leaf function with a 1-unit blast radius** — check that one unit.
- **Anything referenced by >1 unit** — check all of them, or don't rename it.
