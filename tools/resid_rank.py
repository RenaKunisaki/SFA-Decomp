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
`.o` carries no `.text` at all, an attribution artifact; OSTime.c, voice_id.c,
ARWArwing.c, mtx.c and engine/52 leave a handful of bytes each.  Treat that set
as the screen's known noise floor.  If the count grows after a toolchain or
config change, the screen -- not the units -- is what regressed.

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


def canon(rl, sec, remap, fnsym=None):
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
            o = remap(off)
            if o is None:
                continue
            o &= ~3
        out.append((o, typ, c))
    return out


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


def screen(ours, tgt):
    do, so, no, ro, fo = load(ours)
    dt, st, nt, rt, ft = load(tgt)
    bo, remap = text_excision(do.get(".text", b""), so, nt.get(".text", set()))
    lo, lt = fn_at(fo), fn_at(ft)
    per, tot_b, tot_r = {}, 0, 0
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
        nr = len(set(canon(ro.get(sec, []), sec, remap, lo))
                 ^ set(canon(rt.get(sec, []), sec, lambda x: x, lt)))
        if nb or nr:
            per[sec] = (nb, nr, len(a), len(b))
            tot_b += nb
            tot_r += nr
    return tot_b, tot_r, per


def units(only=()):
    cfg = json.load(open(ROOT / "build/GSAE01/config.json"))
    rep = json.load(open(ROOT / "build/GSAE01/report.json"))
    meta = {}
    for u in rep["units"]:
        mod = u["metadata"].get("module_name", "")
        n = u["name"]
        if mod and n.startswith(mod + "/"):
            n = n[len(mod) + 1:]
        meta[n] = (u["metadata"].get("complete", False),
                   u["measures"].get("fuzzy_match_percent", 0.0),
                   sum(1 for f in u.get("functions", [])
                       if f.get("measures", {}).get("fuzzy_match_percent", 0) < 100.0))
    for u in cfg["units"]:
        name = u["name"]
        key = name[:-2] if name.endswith(".c") else name
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
            tb, tr, per = screen(ours, tgt)
        except Exception as e:
            print(f"ERR {name}: {e}", file=sys.stderr)
            continue
        rows.append((tb, tr, name, per, done, s1, fz))
    rows.sort(key=lambda r: (r[0] + 4 * r[1], r[0]))
    print(f"{'resid':>8} {'relocD':>7} {'M':>1} {'sub100':>6} {'fuzzy':>7}  unit")
    for tb, tr, name, per, done, s1, fz in rows:
        if not tb and not tr:
            continue
        parts = " ".join(
            f"{k}:{v[0]}b" + (f"/{v[1]}r" if v[1] else "")
            + (f"(sz {v[2]}!={v[3]})" if v[2] != v[3] else "")
            for k, v in per.items())
        print(f"{tb:>8} {tr:>7} {'Y' if done else '.':>1} {s1:>6} {fz:>7.2f}  {name}"
              f"\n           {parts}")
    clean = [r for r in rows if not r[0] and not r[1]]
    fp = [r for r in rows if (r[0] or r[1]) and r[4]]
    print(f"\n# screened {len(rows)}  link-clean {len(clean)}  differing {len(rows)-len(clean)}")
    print(f"# CALIBRATION complete-but-differing (expect 8, all SDK no-.text units): {len(fp)}")
    for r in fp:
        print(f"    {r[0]}b/{r[1]}r  {r[2]}")
    bad = [r for r in clean if not r[4]]
    print(f"# LINK-CLEAN but NonMatching (flip candidates): {len(bad)}")
    for r in bad:
        print(f"    {r[2]}")


def _aligned(ours, tgt):
    keep = load(tgt)[2].get(".text", set())
    return [r for r in dis(ours) if r[0] in keep], dis(tgt)


def cmd_where(args):
    for name, ours, tgt, _ in units(args):
        da, db = _aligned(ours, tgt)
        print(f"# {name}: ours {len(da)} insns / retail {len(db)}")
        if len(da) != len(db):
            print("#   INSTRUCTION COUNT DIFFERS -- inlining or control-flow shape")
            continue
        n = 0
        for i, (x, y) in enumerate(zip(da, db)):
            if x[1] == y[1]:
                continue
            n += 1
            kind = "reg" if REG.sub("#", x[1]) == REG.sub("#", y[1]) else "STRUCT"
            if n <= 60:
                print(f"  [{x[0]}] insn#{i} ({kind})\n"
                      f"      ours   {x[1]}\n      retail {y[1]}")
        print(f"#   differing instructions: {n}")


def cmd_classify(args):
    out = []
    for name, ours, tgt, (done, fz, s1) in units(args):
        if done:
            continue
        try:
            tb, tr, _ = screen(ours, tgt)
        except Exception:
            continue
        if not tb and not tr:
            continue
        da, db = _aligned(ours, tgt)
        ro = st = 0
        fns = set()
        if len(da) == len(db):
            for x, y in zip(da, db):
                if x[1] == y[1]:
                    continue
                fns.add(x[0])
                if REG.sub("#", x[1]) == REG.sub("#", y[1]):
                    ro += 1
                else:
                    st += 1
        else:
            st = abs(len(da) - len(db)) or 1
            fns.add("<insn-count>")
        kind = ("LEN" if len(da) != len(db) else "STRUCT" if st
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
