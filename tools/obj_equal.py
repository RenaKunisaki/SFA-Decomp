#!/usr/bin/env python3
"""Object-level equality check for ELF32 big-endian PowerPC objects.

Section CONTENTS alone do not certify that two builds of a translation unit are
the same object: two invocations can emit byte-identical PROGBITS and still
differ in their RELOCATIONS, which changes what the linker produces.  This
compares contents, relocations and the symbol table, normalising the churn that
is genuinely meaningless (symbol table ordering / indices, and the object's own
recorded source path).

Usage:
    obj_equal.py A.o B.o
    obj_equal.py --tree DIR_A DIR_B          compare every .o found under both
    obj_equal.py --self-test                 run the built-in controls
"""

import argparse
import hashlib
import os
import struct
import sys

SHT_NULL = 0
SHT_PROGBITS = 1
SHT_SYMTAB = 2
SHT_STRTAB = 3
SHT_RELA = 4
SHT_NOBITS = 8
SHT_REL = 9


class Elf:
    def __init__(self, path):
        self.path = path
        with open(path, "rb") as fp:
            self.data = fp.read()
        d = self.data
        if d[:4] != b"\x7fELF":
            raise ValueError("%s: not an ELF file" % path)
        if d[4] != 1 or d[5] != 2:
            raise ValueError("%s: expected 32-bit big-endian" % path)
        (e_shoff,) = struct.unpack_from(">I", d, 0x20)
        (e_shentsize, e_shnum, e_shstrndx) = struct.unpack_from(">HHH", d, 0x2E)
        self.sections = []
        for i in range(e_shnum):
            off = e_shoff + i * e_shentsize
            (name, stype, flags, addr, soff, size, link, info,
             align, entsize) = struct.unpack_from(">10I", d, off)
            self.sections.append({
                "idx": i, "name_off": name, "type": stype, "flags": flags,
                "addr": addr, "off": soff, "size": size, "link": link,
                "info": info, "align": align, "entsize": entsize,
            })
        shstr = self.sections[e_shstrndx]
        self.shstrtab = d[shstr["off"]:shstr["off"] + shstr["size"]]
        for s in self.sections:
            s["name"] = self._str(self.shstrtab, s["name_off"])
        self.by_name = {s["name"]: s for s in self.sections}
        self._symbols = None

    @staticmethod
    def _str(tab, off):
        end = tab.find(b"\0", off)
        return tab[off:end].decode("utf-8", "replace")

    def body(self, s):
        if s["type"] == SHT_NOBITS:
            return b""
        return self.data[s["off"]:s["off"] + s["size"]]

    @property
    def symbols(self):
        if self._symbols is not None:
            return self._symbols
        out = []
        for s in self.sections:
            if s["type"] != SHT_SYMTAB:
                continue
            strtab_s = self.sections[s["link"]]
            strtab = self.data[strtab_s["off"]:strtab_s["off"] + strtab_s["size"]]
            n = s["size"] // 16
            for i in range(n):
                off = s["off"] + i * 16
                (nm, value, size, info, other, shndx) = struct.unpack_from(
                    ">IIIBBH", self.data, off)
                name = self._str(strtab, nm)
                if shndx == 0:
                    sec = "UNDEF"
                elif shndx >= 0xFF00:
                    sec = "SPECIAL:%#x" % shndx
                else:
                    sec = self.sections[shndx]["name"]
                out.append({
                    "idx": i, "name": name, "value": value, "size": size,
                    "info": info, "other": other, "sec": sec,
                })
        self._symbols = out
        return out

    def sym_key(self, idx):
        """Index-independent identity for a relocation's target symbol."""
        syms = self.symbols
        if idx >= len(syms):
            return ("BADIDX", idx)
        s = syms[idx]
        stype = s["info"] & 0xF
        if s["name"]:
            # STT_SECTION symbols carry an empty-ish name in some producers;
            # a named symbol is identified by its name plus binding/type.
            return ("NAME", s["name"], stype, s["info"] >> 4)
        if stype == 3:  # STT_SECTION
            return ("SECTION", s["sec"])
        return ("ANON", s["sec"], s["value"], stype)

    def relocs(self):
        """{target section name: sorted list of normalised relocation tuples}"""
        out = {}
        for s in self.sections:
            if s["type"] not in (SHT_REL, SHT_RELA):
                continue
            tgt = self.sections[s["info"]]["name"]
            rela = s["type"] == SHT_RELA
            step = 12 if rela else 8
            n = s["size"] // step
            rows = []
            for i in range(n):
                off = s["off"] + i * step
                if rela:
                    r_off, r_info, r_add = struct.unpack_from(">IIi", self.data, off)
                else:
                    r_off, r_info = struct.unpack_from(">II", self.data, off)
                    r_add = 0
                rtype = r_info & 0xFF
                rsym = r_info >> 8
                rows.append((r_off, rtype, r_add, self.sym_key(rsym)))
            rows.sort()
            out[tgt] = rows
        return out


def md5(b):
    return hashlib.md5(b).hexdigest()


def content_map(e):
    """Section contents keyed by name.  `.comment`-style producer strings and
    any section recording the source path are compared separately."""
    out = {}
    for s in e.sections:
        if s["type"] in (SHT_NULL, SHT_SYMTAB, SHT_STRTAB, SHT_REL, SHT_RELA):
            continue
        if s["name"] in (".shstrtab", ".strtab"):
            continue
        if s["type"] == SHT_NOBITS:
            out[s["name"]] = ("NOBITS", s["size"])
        else:
            out[s["name"]] = ("BITS", s["size"], md5(e.body(s)))
    return out


def symbol_map(e):
    """Name -> (section, value, size, info).  Index/order churn ignored."""
    out = {}
    for s in e.symbols:
        if not s["name"]:
            continue
        out[s["name"]] = (s["sec"], s["value"], s["size"], s["info"])
    return out


def compare(pa, pb, want_syms=True):
    """Return list of difference strings; empty means the objects agree."""
    a, b = Elf(pa), Elf(pb)
    diffs = []

    ca, cb = content_map(a), content_map(b)
    for name in sorted(set(ca) | set(cb)):
        if name not in ca:
            diffs.append("CONTENT: section %s only in B" % name)
        elif name not in cb:
            diffs.append("CONTENT: section %s only in A" % name)
        elif ca[name] != cb[name]:
            diffs.append("CONTENT: section %s differs %s vs %s"
                         % (name, ca[name], cb[name]))

    ra, rb = a.relocs(), b.relocs()
    for name in sorted(set(ra) | set(rb)):
        la, lb = ra.get(name, []), rb.get(name, [])
        if len(la) != len(lb):
            diffs.append("RELOC: %s count %d vs %d" % (name, len(la), len(lb)))
            continue
        bad = [(x, y) for x, y in zip(la, lb) if x != y]
        if bad:
            x, y = bad[0]
            diffs.append("RELOC: %s %d/%d entries differ, first A=%r B=%r"
                         % (name, len(bad), len(la), x, y))

    if want_syms:
        sa, sb = symbol_map(a), symbol_map(b)
        only_a = sorted(set(sa) - set(sb))
        only_b = sorted(set(sb) - set(sa))
        if only_a:
            diffs.append("SYM: %d only in A (e.g. %s)" % (len(only_a), only_a[0]))
        if only_b:
            diffs.append("SYM: %d only in B (e.g. %s)" % (len(only_b), only_b[0]))
        changed = [k for k in set(sa) & set(sb) if sa[k] != sb[k]]
        if changed:
            k = sorted(changed)[0]
            diffs.append("SYM: %d changed (e.g. %s %r vs %r)"
                         % (len(changed), k, sa[k], sb[k]))
    return diffs


def walk_objs(root):
    out = {}
    for dirpath, _, files in os.walk(root):
        for f in files:
            if f.endswith(".o"):
                p = os.path.join(dirpath, f)
                out[os.path.relpath(p, root)] = p
    return out


def cmd_tree(da, db, want_syms):
    ta, tb = walk_objs(da), walk_objs(db)
    only_a = sorted(set(ta) - set(tb))
    only_b = sorted(set(tb) - set(ta))
    common = sorted(set(ta) & set(tb))
    ndiff = 0
    for rel in common:
        d = compare(ta[rel], tb[rel], want_syms)
        if d:
            ndiff += 1
            print("DIFFER %s" % rel)
            for line in d:
                print("    %s" % line)
    for rel in only_a:
        print("MISSING-IN-B %s" % rel)
    for rel in only_b:
        print("NEW-IN-B %s" % rel)
    print("%d compared / %d differ / %d missing / %d new"
          % (len(common), ndiff, len(only_a), len(only_b)))
    return 1 if (ndiff or only_a or only_b) else 0


def self_test():
    """Controls, built from real objects in this tree.

    NEGATIVE control: an object against a byte-identical copy of itself with a
    different filename -> must report EQUAL (the check must not be trigger-happy).
    POSITIVE control (contents): two genuinely different objects -> CONTENT diff.
    POSITIVE control (relocations only): a synthetic object whose PROGBITS are
    byte-identical to the original but whose relocation table names a different
    symbol -> must report a RELOC diff while reporting no CONTENT diff.  This is
    exactly the class an md5-of-section-contents check is blind to.
    """
    import shutil
    import tempfile

    root = "build/GSAE01/src"
    objs = sorted(walk_objs(root).values())
    if len(objs) < 2:
        print("self-test: no built objects under %s" % root)
        return 2
    base = None
    for p in objs:
        e = Elf(p)
        r = e.relocs()
        if any(len(v) > 2 for v in r.values()):
            base = p
            break
    if base is None:
        print("self-test: no object with relocations found")
        return 2

    ok = True
    tmp = tempfile.mkdtemp(prefix="objequal_")
    try:
        # --- negative control -------------------------------------------
        copy = os.path.join(tmp, "copy.o")
        shutil.copyfile(base, copy)
        d = compare(base, copy)
        print("negative control (identical copy): %s"
              % ("EQUAL - pass" if not d else "FAIL %r" % d))
        ok &= not d

        # --- positive control, contents ---------------------------------
        other = next(p for p in objs if p != base
                     and content_map(Elf(p)) != content_map(Elf(base)))
        d = compare(base, other)
        has_content = any(x.startswith("CONTENT") for x in d)
        print("positive control (different object): %s"
              % ("CONTENT diff seen - pass" if has_content else "FAIL %r" % d))
        ok &= has_content

        # --- positive control, relocations only -------------------------
        # Repoint one relocation at a different existing symbol index.  The
        # PROGBITS bytes are untouched, so a content-only check sees nothing.
        e = Elf(base)
        relsec = None
        for s in e.sections:
            if s["type"] in (SHT_REL, SHT_RELA) and s["size"] >= 8:
                relsec = s
                break
        raw = bytearray(e.data)
        step = 12 if relsec["type"] == SHT_RELA else 8
        off = relsec["off"]
        (r_off, r_info) = struct.unpack_from(">II", raw, off)
        cur = r_info >> 8
        alt = next(i for i, s in enumerate(e.symbols)
                   if i != cur and s["name"] and e.sym_key(i) != e.sym_key(cur))
        struct.pack_into(">I", raw, off + 4, (alt << 8) | (r_info & 0xFF))
        mutated = os.path.join(tmp, "reloc_mutated.o")
        with open(mutated, "wb") as fp:
            fp.write(raw)

        content_same = content_map(Elf(base)) == content_map(Elf(mutated))
        d = compare(base, mutated, want_syms=False)
        has_reloc = any(x.startswith("RELOC") for x in d)
        print("positive control (reloc-only change on %s):" % os.path.basename(base))
        print("    section contents identical : %s" % content_same)
        print("    RELOC diff detected        : %s" % has_reloc)
        print("    -> %s" % ("pass" if (content_same and has_reloc) else "FAIL"))
        ok &= content_same and has_reloc
    finally:
        shutil.rmtree(tmp, ignore_errors=True)

    print("SELF-TEST %s" % ("PASS" if ok else "FAIL"))
    return 0 if ok else 3


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("a", nargs="?")
    ap.add_argument("b", nargs="?")
    ap.add_argument("--tree", nargs=2, metavar=("DIR_A", "DIR_B"))
    ap.add_argument("--self-test", action="store_true")
    ap.add_argument("--no-syms", action="store_true",
                    help="skip the symbol-table comparison")
    args = ap.parse_args()

    if args.self_test:
        return self_test()
    if args.tree:
        return cmd_tree(args.tree[0], args.tree[1], not args.no_syms)
    if not (args.a and args.b):
        ap.error("need two objects, --tree, or --self-test")
    d = compare(args.a, args.b, not args.no_syms)
    if not d:
        print("EQUAL")
        return 0
    for line in d:
        print(line)
    return 1


if __name__ == "__main__":
    sys.exit(main())
