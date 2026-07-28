"""Rank NonMatching units by REAL RESIDUAL BYTES -- what mwld still has to reconcile.

`report.json` `fuzzy_match_percent` is the project's match-percentage truth, but
it is blind to emission ORDER, `.data` layout and pool naming, so it is the wrong
worklist for deciding which unit is closest to linking clean.  This tool screens
our `.o` against the retail `.o` over raw section bytes plus the full relocation
list, applying only the normalisations that genuine link-equivalence permits:

  1. locally-defined reloc targets canonicalise to (section, value+addend), so
     anonymous `@N` pool symbols compare equal to retail's named `lbl_` ones;
  2. external reloc targets canonicalise to their linked DOL address, so
     `gFooHi+4` and `gFooLo+0` -- literally the same address after link --
     compare equal;
  3. `.text` reloc offsets round down to the containing instruction word, since
     MWCC records a halfword reloc two bytes into the instruction dtk names by
     its word address;
  0. `.text` compares FUNCTION-ALIGNED: every retail function against our copy
     of the SAME symbol, summing the per-function differing bytes.  Comparing
     `.text` at absolute section offsets let a single short function shift every
     byte behind it, so the entire tail of the section counted as residual --
     the absolute figure mostly ranked WHERE in `.text` the first size defect
     sat, not how much was wrong (engine/0 read 56874 absolute against 3166
     function-aligned, player.c 88171 against 10018).  A genuine size difference
     still counts, as the per-function length delta, because it is still a real
     defect that has to be fixed before the unit can link clean; the absolute
     figure stays in the `abs` column and in `[abs N]` per section.  `.text`
     reloc OFFSETS canonicalise the same way, to (owning function, offset within
     it), for the same reason;
  3b. a reloc TARGETING `.text` canonicalises to (containing function, offset
     within that function) rather than to a raw `.text` section offset.  A
     function-pointer table entry names a function, not an address: mwld
     resolves it to wherever that function lands, so the entry is link-identical
     to retail's whenever it names the same function at the same intra-function
     offset.  Comparing raw section offsets instead made every such reloc flip
     the moment ANY earlier function in our `.text` differed in size -- a whole
     `.data` jumptable reported as residual on the strength of one unrelated
     `.text` byte, double-counting a defect the `.text` column already carries.
     Only `.text` targets get this; `.data`/`.rodata`/`.sdata2` targets keep
     comparing by absolute offset, because for those sections the layout IS the
     linked image and a shifted target is a real difference;
  4. static functions we define that retail's `.o` lacks are excised (mwld
     dead-strips unreferenced statics, so they can never appear in a
     dtk-reconstructed object), and every reloc offset and `.text`-targeting
     reloc value is remapped through that excision;
  5. trailing all-zero alignment padding is excluded;
  6. relocations compare as SETS, so the order in which a `.rela` section lists
     them is inert.  mwld consumes a relocation section as an unordered pool, so
     the order MWCC emits in versus the offset-sorted order dtk reconstructs is
     not a difference at all.

Everything else -- code emission order, `.data` layout, section content and size
-- is counted, because all of it changes the linked image.

CALIBRATION.  Every unit `report.json` marks `complete` is provably link-equal,
because dtk gates the DOL sha1 on it.  The screen must therefore report zero
residual for all of them, and `rank` prints that count.  It currently reads 8
out of 998 (99.2% clean), all of them SDK/library units rather than game code:
OS.c, OSExec.c and synth_seq_queue.c have residual only because their retail
`.o` carries no `.text` at all, an attribution artifact (function-aligned they
read 0, 14 and 0 bytes); OSTime.c, voice_id.c, ARWArwing.c, mtx.c and
__exception.s leave a handful of bytes each.  Treat that set as the screen's
known noise floor.  If the count grows after a toolchain or config change, the
screen -- not the units -- is what regressed.  Note the noise floor is measured
on the ABSOLUTE figure, because link-equality is byte-equality including layout;
the function-aligned figure is the worklist, not the flip test.

The report-to-config key normalisation has to cope with two shapes dtk emits:
`metadata.module_name` is absent for some units (their `main/` prefix has to be
stripped by matching against the known module names instead), and the report
strips every source suffix, not just `.c`.  Getting either wrong silently
defaults the unit to NonMatching and manufactures phantom flip candidates --
`__start.c`, `__mem.c`, `mem_TRK.c`, `__init_cpp_exceptions.cpp` and
`targsupp.s` were all reported as link-clean flip candidates while already
being `complete`.

ZERO RESIDUAL IS NECESSARY, NOT SUFFICIENT.  `rank` splits its zero-residual
units into flip candidates and a BLOCKED list.  A blocked unit is byte- and
relocation-equal but its retail `.o` exports a global name ours does not --
invariably a `lbl_` pool constant, which MWCC emits as an anonymous local `@N`
-- and some sibling that is still on its retail object references that name.
Flipping one links but fails `undefined:`; it only becomes flippable once every
such sibling matches too.

WARNING.  Residual and `fuzzy_match_percent` can move in OPPOSITE directions: a
change may cut residual while regressing fuzzy.  Only a change that drives
residual to zero flips a unit to `complete`; anything short of that must not
regress fuzzy.  Always regenerate `report.json` and read both numbers before
believing a win.

Subcommands
    rank [unit ...]     residual ranking, largest-first, with calibration trailer
    where <unit>        locate the residual: which function, which instruction
    classify            split residuals into REGROT (allocator/coloring caps)
                        vs STRUCT (different instruction sequence -- attackable)
    order               units whose functions are a PERMUTATION of retail's
                        order: the target of the emission-order lever
"""
import json
import os
import re
import subprocess
import sys
from pathlib import Path

from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection
from elftools.elf.relocation import RelocationSection

ROOT = Path(os.environ.get("SFA_ROOT", Path(__file__).resolve().parents[1]))
OBJDUMP = ROOT / "build/binutils/powerpc-eabi-objdump"
SECS = [".text", ".data", ".rodata", ".sdata", ".sdata2", ".sbss", ".bss",
        ".ctors", ".dtors", ".init", ".extab", ".extabindex"]
SYMRE = re.compile(r"^\s*(\S+)\s*=\s*\S*?:?(0x[0-9A-Fa-f]+)")
REG = re.compile(r"\br\d+\b|\bf\d+\b|\bcr\d+\b")

GADDR = {}
for _p in sorted(ROOT.glob("config/GSAE01/**/symbols.txt")):
    for _line in open(_p, errors="ignore"):
        _m = SYMRE.match(_line)
        if _m:
            GADDR.setdefault(_m.group(1), int(_m.group(2), 16))


def load(path):
    with open(path, "rb") as f:
        elf = ELFFile(f)
        sname = {i: s.name for i, s in enumerate(elf.iter_sections())}
        data, statics, names, funcs = {}, [], {}, []
        for sec in elf.iter_sections():
            if sec.name in SECS:
                data[sec.name] = (b"\0" * sec["sh_size"]
                                  if sec["sh_type"] == "SHT_NOBITS" else sec.data())
        for sec in elf.iter_sections():
            if not isinstance(sec, SymbolTableSection):
                continue
            for s in sec.iter_symbols():
                sh = s["st_shndx"]
                if not isinstance(sh, int):
                    continue
                sn = sname.get(sh)
                if sn not in SECS or not s.name:
                    continue
                names.setdefault(sn, set()).add(s.name)
                if sn == ".text" and s["st_info"]["type"] == "STT_FUNC":
                    funcs.append((s["st_value"], s["st_size"], s.name))
                    if s["st_size"] and s["st_info"]["bind"] == "STB_LOCAL":
                        statics.append((s.name, s["st_value"], s["st_size"]))
        rel = {}
        for sec in elf.iter_sections():
            if not isinstance(sec, RelocationSection):
                continue
            tgt = sname.get(sec["sh_info"])
            if tgt not in SECS:
                continue
            st = elf.get_section(sec["sh_link"])
            for r in sec.iter_relocations():
                sym = st.get_symbol(r["r_info_sym"])
                add = r["r_addend"] if "r_addend" in r.entry else 0
                sh = sym["st_shndx"]
                tsec = sname.get(sh) if isinstance(sh, int) else None
                rel.setdefault(tgt, []).append(
                    (r["r_offset"], r["r_info_type"], tsec,
                     sym["st_value"], sym.name, add))
        return data, statics, names, rel, funcs


def fn_at(funcs):
    """Map a `.text` offset to (containing function, offset within it)."""
    tab = sorted(set(funcs))
    starts = [f[0] for f in tab]

    def lookup(v):
        import bisect
        i = bisect.bisect_right(starts, v) - 1
        while i >= 0:
            st, sz, nm = tab[i]
            end = st + sz if sz else (starts[i + 1] if i + 1 < len(tab) else None)
            if end is None or v < end:
                return (nm, v - st)
            i -= 1
        return None
    return lookup


def fn_spans(funcs, size):
    """name -> (start, end) byte span of each function body in `.text`."""
    tab = sorted(set(funcs))
    out = {}
    for i, (v, sz, nm) in enumerate(tab):
        end = v + sz if sz else (tab[i + 1][0] if i + 1 < len(tab) else size)
        out.setdefault(nm, (v, min(end, size)))
    return out


def text_fn_resid(ao, at, fo, ft):
    """Differing bytes counted FUNCTION-ALIGNED: every retail function against
    OUR copy of the same symbol.

    Comparing `.text` at absolute section offsets makes one short function shift
    every byte behind it, so the whole tail of the section counts as residual --
    the absolute figure mostly ranks WHERE in `.text` the first size defect sits,
    not how much is actually wrong.  Matching by symbol first isolates each
    defect to the function that owns it.  A genuine size difference still counts
    (as the length delta), because it is still a real defect that has to be fixed
    before the unit can link clean.
    """
    so_ = fn_spans(fo, len(ao))
    st_ = fn_spans(ft, len(at))
    nb = 0
    for nm, (s, e) in st_.items():
        b = at[s:e]
        if nm not in so_:
            nb += len(b) if any(b) else 0
            continue
        s2, e2 = so_[nm]
        a = ao[s2:e2]
        n = min(len(a), len(b))
        nb += sum(1 for i in range(n) if a[i] != b[i]) + abs(len(a) - len(b))
    return nb


def canon(rl, sec, remap, fnsym=None, offkey=None):
    out = []
    for off, typ, tsec, val, nm, add in rl:
        if tsec in SECS:
            t = val + add
            if tsec == ".text":
                if remap(t) is None:
                    continue
                sym = fnsym(t) if fnsym else None
                t = sym if sym is not None else ("ABS", remap(t))
            c = ("DEF", tsec, t)
        elif nm in GADDR:
            c = ("ADDR", GADDR[nm] + add)
        else:
            c = ("UND", nm, add)
        o = off
        if sec == ".text":
            if offkey:
                o = offkey(off)
            else:
                o = remap(off)
                o = None if o is None else ("ABS", o & ~3)
            if o is None:
                continue
        out.append((o, typ, c))
    return out


def text_offkey(lookup, keep):
    """A `.text` reloc offset as (owning function, offset within it).

    Same reason as the byte comparison: an absolute offset makes every reloc
    behind a size defect report as residual.  A reloc lives inside a function,
    and mwld places it wherever that function lands.
    """
    def key(off):
        r = lookup(off)
        if r is None:
            return None
        nm, d = r
        return None if nm not in keep else (nm, d & ~3)
    return key


def text_excision(bo, statics, keep):
    """Byte mask + offset remap for the statics mwld would dead-strip."""
    drop = set()
    for n, v, s in statics:
        if n in keep:
            continue
        e = v + s
        while e < len(bo) and e % 4:
            e += 1
        drop |= set(range(v, min(e, len(bo))))
    if not drop:
        return bo, (lambda x: x)
    new, k = {}, 0
    for i in range(len(bo)):
        if i in drop:
            continue
        new[i] = k
        k += 1
    new[len(bo)] = k
    return bytes(b for i, b in enumerate(bo) if i not in drop), new.get


def exports(path):
    """Global symbol names the object DEFINES -- what mwld can resolve from it."""
    out = set()
    with open(path, "rb") as f:
        elf = ELFFile(f)
        sn = {i: s.name for i, s in enumerate(elf.iter_sections())}
        for sec in elf.iter_sections():
            if not isinstance(sec, SymbolTableSection):
                continue
            for s in sec.iter_symbols():
                sh = s["st_shndx"]
                if not isinstance(sh, int) or sn.get(sh) not in SECS:
                    continue
                if (s.name and s["st_info"]["bind"] != "STB_LOCAL"
                        and s["st_info"]["type"] != "STT_SECTION"
                        and not s.name.startswith("gap_")):
                    out.add(s.name)
    return out


def missing_exports(ours, tgt):
    """Names retail's object exports that ours does not.

    A unit can screen link-clean and still break the link: MWCC emits pool
    constants as anonymous local @N symbols, so a `lbl_...` that retail's
    object exports globally vanishes when we start linking ours.  Any sibling
    still on its retail object references that name and mwld fails undefined.
    The unit only becomes flippable once every such sibling matches too.
    """
    return exports(tgt) - exports(ours)


def screen(ours, tgt):
    do, so, no, ro, fo = load(ours)
    dt, st, nt, rt, ft = load(tgt)
    bo, remap = text_excision(do.get(".text", b""), so, nt.get(".text", set()))
    lo, lt = fn_at(fo), fn_at(ft)
    common = ({n for _, _, n in fo} & {n for _, _, n in ft})
    ko, kt = text_offkey(lo, common), text_offkey(lt, common)
    per, tot_b, tot_r, tot_abs = {}, 0, 0, 0
    for sec in SECS:
        a = bo if sec == ".text" else do.get(sec, b"")
        b = dt.get(sec, b"")
        nb = 0
        if len(a) != len(b):
            n = min(len(a), len(b))
            tail = (a if len(a) > len(b) else b)[n:]
            nb = (0 if not any(tail) else len(tail))
            nb += sum(1 for i in range(n) if a[i] != b[i])
        elif a != b:
            nb = sum(1 for x, y in zip(a, b) if x != y)
        na = nb
        if sec == ".text":
            nb = text_fn_resid(do.get(".text", b""), b, fo, ft)
        nr = len(set(canon(ro.get(sec, []), sec, remap, lo, ko if sec == ".text" else None))
                 ^ set(canon(rt.get(sec, []), sec, lambda x: x, lt,
                             kt if sec == ".text" else None)))
        if nb or nr or na:
            per[sec] = (nb, nr, len(a), len(b), na)
            tot_b += nb
            tot_r += nr
            tot_abs += na
    return tot_b, tot_r, per, tot_abs


def units(only=()):
    cfg = json.load(open(ROOT / "build/GSAE01/config.json"))
    rep = json.load(open(ROOT / "build/GSAE01/report.json"))
    meta = {}
    modules = {m["name"] for m in cfg.get("modules", [])} | {cfg["name"]}
    for u in rep["units"]:
        mod = u["metadata"].get("module_name") or ""
        n = u["name"]
        if mod and n.startswith(mod + "/"):
            n = n[len(mod) + 1:]
        row = (u["metadata"].get("complete", False),
               u["measures"].get("fuzzy_match_percent", 0.0),
               sum(1 for f in u.get("functions", [])
                   if f.get("measures", {}).get("fuzzy_match_percent", 0) < 100.0))
        meta[n] = row
        head = n.split("/", 1)
        if len(head) == 2 and head[0] in modules:
            meta.setdefault(head[1], row)
    for u in cfg["units"]:
        name = u["name"]
        key = re.sub(r"\.(c|cpp|cp|cxx|cc|s|S)$", "", name)
        if only and not any(o in name for o in only):
            continue
        tgt = ROOT / u["object"]
        ours = ROOT / u["object"].replace("build/GSAE01/obj/", "build/GSAE01/src/")
        if ours.exists() and tgt.exists():
            yield name, str(ours), str(tgt), meta.get(key, (False, 0.0, 0))


def norm_insn(t):
    """Drop absolute branch targets; the <sym+0xNN> form is already relative."""
    return t.split()[0] + " " + t[t.index("<"):] if "<" in t else t


def dis(obj, sec=".text"):
    out = subprocess.run([str(OBJDUMP), "-M", "gekko", "-drz", "-j", sec, str(obj)],
                         capture_output=True, text=True).stdout
    rows, cur = [], "?"
    for line in out.splitlines():
        if ">:" in line and line and line[0].isdigit():
            cur = line.split("<")[1].split(">")[0]
            continue
        p = line.split("\t")
        if len(p) >= 3:
            try:
                int(p[0].strip().rstrip(":"), 16)
            except ValueError:
                continue
            rows.append((cur, norm_insn(p[2].strip())))
    return rows


def fn_order(path, sec=".text"):
    with open(path, "rb") as f:
        elf = ELFFile(f)
        sn = {i: s.name for i, s in enumerate(elf.iter_sections())}
        o = []
        for s_ in elf.iter_sections():
            if isinstance(s_, SymbolTableSection):
                for s in s_.iter_symbols():
                    sh = s["st_shndx"]
                    if (isinstance(sh, int) and sn.get(sh) == sec and s.name
                            and s["st_info"]["type"] == "STT_FUNC"):
                        o.append((s["st_value"], s.name))
        return [n for _, n in sorted(set(o))]


def cmd_rank(args):
    rows = []
    for name, ours, tgt, (done, fz, s1) in units(args):
        try:
            tb, tr, per, ta = screen(ours, tgt)
        except Exception as e:
            print(f"ERR {name}: {e}", file=sys.stderr)
            continue
        rows.append((tb, tr, name, per, done, s1, fz, ta))
    rows.sort(key=lambda r: (r[0] + 4 * r[1], r[7]))
    print(f"{'resid':>8} {'abs':>8} {'relocD':>7} {'M':>1} {'sub100':>6} {'fuzzy':>7}  unit")
    for tb, tr, name, per, done, s1, fz, ta in rows:
        if not tb and not tr and not ta:
            continue
        parts = " ".join(
            f"{k}:{v[0]}b" + (f"[abs {v[4]}]" if v[4] != v[0] else "")
            + (f"/{v[1]}r" if v[1] else "")
            + (f"(sz {v[2]}!={v[3]})" if v[2] != v[3] else "")
            for k, v in per.items())
        print(f"{tb:>8} {ta:>8} {tr:>7} {'Y' if done else '.':>1} {s1:>6} {fz:>7.2f}  {name}"
              f"\n           {parts}")
    clean = [r for r in rows if not r[0] and not r[1] and not r[7]]
    fp = [r for r in rows if (r[0] or r[1] or r[7]) and r[4]]
    print(f"\n# screened {len(rows)}  link-clean {len(clean)}  differing {len(rows)-len(clean)}")
    print(f"# CALIBRATION complete-but-differing (expect 8, all SDK no-.text units): {len(fp)}")
    for r in fp:
        print(f"    {r[0]}b[abs {r[7]}]/{r[1]}r  {r[2]}")
    bad = [r for r in clean if not r[4]]
    ready, held = [], []
    lookup = {n: (o, t) for n, o, t, _ in units(args)}
    for r in bad:
        miss = missing_exports(*lookup[r[2]])
        (held if miss else ready).append((r[2], sorted(miss)))
    print(f"# LINK-CLEAN but NonMatching (flip candidates): {len(ready)}")
    for n, _ in ready:
        print(f"    {n}")
    if held:
        print(f"# LINK-CLEAN but BLOCKED (retail object exports names ours does "
              f"not; siblings still reference them): {len(held)}")
        for n, miss in held:
            print(f"    {n}  missing: {' '.join(miss)}")


def _bysym(rows):
    out = {}
    for fn, insn in rows:
        out.setdefault(fn, []).append(insn)
    return out


def _aligned(ours, tgt):
    """Per-symbol instruction lists, so a length defect in one function cannot
    misalign every function behind it."""
    a, b = _bysym(dis(ours)), _bysym(dis(tgt))
    return {k: v for k, v in a.items() if k in b}, b


def cmd_where(args):
    for name, ours, tgt, _ in units(args):
        da, db = _aligned(ours, tgt)
        n = shown = 0
        lens = []
        for fn in sorted(db, key=lambda f: (f not in da, f)):
            x, y = da.get(fn, []), db[fn]
            if x == y:
                continue
            if len(x) != len(y):
                lens.append(f"{fn} ({len(x)}!={len(y)})")
                continue
            for i, (p, q) in enumerate(zip(x, y)):
                if p == q:
                    continue
                n += 1
                kind = "reg" if REG.sub("#", p) == REG.sub("#", q) else "STRUCT"
                shown += 1
                if shown <= 60:
                    print(f"  [{fn}] insn#{i} ({kind})\n"
                          f"      ours   {p}\n      retail {q}")
        print(f"# {name}: differing instructions {n} in same-length functions")
        if lens:
            print(f"#   LENGTH DIFFERS (inlining or control-flow shape) in "
                  f"{len(lens)}: {' '.join(lens[:20])}")


def cmd_classify(args):
    out = []
    for name, ours, tgt, (done, fz, s1) in units(args):
        if done:
            continue
        try:
            tb, tr, _, ta = screen(ours, tgt)
        except Exception:
            continue
        if not tb and not tr and not ta:
            continue
        da, db = _aligned(ours, tgt)
        ro = st = ln = 0
        fns = set()
        for fn, y in db.items():
            x = da.get(fn)
            if x == y:
                continue
            fns.add(fn)
            if x is None or len(x) != len(y):
                ln += abs(len(x or ()) - len(y)) or 1
                continue
            for p, q in zip(x, y):
                if p == q:
                    continue
                if REG.sub("#", p) == REG.sub("#", q):
                    ro += 1
                else:
                    st += 1
        kind = ("LEN" if ln else "STRUCT" if st
                else "REGROT" if ro else "DATA")
        out.append((tb + 4 * tr, tb, tr, kind, ro, st, name, ",".join(sorted(fns)[:3])))
    out.sort()
    print(f"{'score':>7} {'resid':>7} {'relD':>5} {'kind':>6} {'reg':>4} {'str':>5}  unit")
    for s, tb, tr, k, ro, st, n, f in out:
        print(f"{s:>7} {tb:>7} {tr:>5} {k:>6} {ro:>4} {st:>5}  {n}\n            {f}")
    from collections import Counter
    print("\n#", Counter(o[3] for o in out))
    print("# REGROT = allocator/coloring cap.  STRUCT/LEN = different instruction"
          " sequence, the class source levers can actually move.")


def cmd_order(args):
    import difflib
    for name, ours, tgt, (done, _, _) in units(args):
        if done:
            continue
        try:
            a, b = fn_order(ours), fn_order(tgt)
        except Exception:
            continue
        a = [x for x in a if x in set(b)]
        if a != b and sorted(a) == sorted(b):
            print(f"ORDER-PERMUTED  {name}  ({len(b)} fns)")
            for l in difflib.unified_diff(a, b, lineterm="", n=1):
                if l[:1] in "+-" and l[:3] not in ("---", "+++"):
                    print("   ", l)


def main():
    cmds = {"rank": cmd_rank, "where": cmd_where,
            "classify": cmd_classify, "order": cmd_order}
    if len(sys.argv) < 2 or sys.argv[1] not in cmds:
        print(__doc__)
        sys.exit(2)
    cmds[sys.argv[1]](tuple(sys.argv[2:]))


if __name__ == "__main__":
    main()
