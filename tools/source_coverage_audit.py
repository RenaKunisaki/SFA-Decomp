#!/usr/bin/env python3
"""Which source files the build compiles, and which scanners can see them.

WHY THIS EXISTS
    Nearly every source-level screen in tools/ selects its population by
    walking src/ and filtering on a file extension.  That filter is a guess
    about what the build compiles, and it has been wrong in both directions at
    once:

      * BLIND SPOTS.  A `.c`-only walk misses three units the build really
        compiles -- Runtime.PPCEABI.H/__init_cpp_exceptions.cpp and the two
        assembled TRK_MINNOW_DOLPHIN `.s` files.  A screen that never reads a
        compiled unit cannot report anything about it, and "we found nothing
        there" then means only "we never looked".

      * FALSE POPULATION.  66 source files sit in the tree that the build never
        compiles at all.  A walk that reads them spends its rows on text that
        cannot reach the DOL, and -- worse -- a later reader can cite one as
        evidence about the binary.  That already happened:
        src/dolphin/os/__ppc_eabi_init.cpp is one of the 66, its
        __init_hardware carries three `bl`s where retail's .init:0x80003354
        carries two, and it was cited as proof that __OSFPRInit is called.  It
        is not; the compiled __init_hardware is in Runtime.PPCEABI.H/__start.c.

    The authority on the population is build.ninja, which configure.py
    generates from the Object() lists.  This module makes that authority
    importable so a screen states its exclusion instead of inheriting one.

WHAT IT IS NOT
    It does not say a scanner SHOULD widen its filter.  A C-shape screen has
    nothing to say about a `.s` file, and adding `.cp` to a C screen pulls in
    four uncompiled Metrowerks C++ sources under an exempt root.  It says only
    what each filter excludes, so the exclusion is a recorded decision.

Usage:
    source_coverage_audit.py             the partition, then the filter matrix
    source_coverage_audit.py --orphans   list the uncompiled sources
    source_coverage_audit.py --self-test controls, both directions
"""

import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NINJA = os.path.join(REPO, "build.ninja")

SOURCE_EXTS = (".c", ".cpp", ".cp", ".s", ".S")
RE_SRC = re.compile(r"\bsrc/[A-Za-z0-9_./+-]+\.(?:c|cpp|cp|s|S)\b")
RE_ENDSWITH = re.compile(r"endswith\(\s*(\([^)]*\)|\"[^\"]*\"|'[^']*')")
RE_RGLOB = re.compile(r"\.rglob\(\s*[\"']\*?([^\"']*)[\"']")
RE_GLOB = re.compile(r"glob\.glob\(\s*[^)]*?\*(\.[A-Za-z]+)")
RE_WALKS = re.compile(r"os\.walk\(|\.rglob\(|glob\.glob\(")

_CACHE = {}


def compiled_sources():
    """Absolute paths of every source file the build compiles.

    Read out of build.ninja rather than configure.py: the Object() lists are
    spread over conditionals and generated DLL scaffolding, and the ninja file
    is what the build actually obeys.
    """
    if "compiled" not in _CACHE:
        if not os.path.exists(NINJA):
            raise SystemExit("build.ninja is missing -- run configure.py first")
        with open(NINJA, encoding="utf-8", errors="replace") as fp:
            text = fp.read()
        _CACHE["compiled"] = {os.path.join(REPO, p) for p in RE_SRC.findall(text)}
    return _CACHE["compiled"]


def on_disk_sources():
    """Absolute paths of every source file under src/, compiled or not."""
    if "ondisk" not in _CACHE:
        found = set()
        for dirpath, _dirs, files in os.walk(os.path.join(REPO, "src")):
            for f in files:
                if os.path.splitext(f)[1] in SOURCE_EXTS:
                    found.add(os.path.join(dirpath, f))
        _CACHE["ondisk"] = found
    return _CACHE["ondisk"]


def orphan_sources():
    """On disk, never compiled.  A row here cannot reach the DOL."""
    return on_disk_sources() - compiled_sources()


def rel(path):
    return os.path.relpath(path, REPO)


def by_ext(paths):
    counts = {}
    for p in paths:
        counts[os.path.splitext(p)[1]] = counts.get(os.path.splitext(p)[1], 0) + 1
    return dict(sorted(counts.items()))


def scanner_filters():
    """Every tool that walks the filesystem, and the extensions it accepts.

    Static read of the source: a filter that is spelled as a literal is the
    filter, and a tool whose population comes from an argument is reported with
    an empty set rather than guessed at.
    """
    rows = []
    for dirpath, _dirs, files in os.walk(os.path.join(REPO, "tools")):
        if "shims" in dirpath:
            continue
        for f in sorted(files):
            if not f.endswith(".py"):
                continue
            p = os.path.join(dirpath, f)
            with open(p, encoding="utf-8", errors="replace") as fp:
                text = fp.read()
            if p == os.path.abspath(__file__):
                continue
            if not RE_WALKS.search(text):
                continue
            exts = set()
            for m in RE_ENDSWITH.finditer(text):
                for e in re.findall(r"[\"']([^\"']+)[\"']", m.group(1)):
                    if e.startswith(".") and len(e) <= 5:
                        exts.add(e)
            for m in RE_RGLOB.finditer(text):
                if m.group(1).startswith("."):
                    exts.add(m.group(1))
            exts.update(RE_GLOB.findall(text))
            rows.append((rel(p), exts))
    return rows


def report():
    comp, disk, orph = compiled_sources(), on_disk_sources(), orphan_sources()
    print("THE POPULATION")
    print("  compiled by the build : %4d  %s" % (len(comp), by_ext(comp)))
    print("  on disk under src/    : %4d  %s" % (len(disk), by_ext(disk)))
    print("  never compiled        : %4d  %s" % (len(orph), by_ext(orph)))

    non_c = sorted(p for p in comp if not p.endswith(".c"))
    print("\nCOMPILED UNITS A `.c`-ONLY WALK CANNOT SEE: %d" % len(non_c))
    for p in non_c:
        print("      %s" % rel(p))

    print("\nSCANNERS THAT WALK THE FILESYSTEM, BY WHAT THEY ACCEPT")
    src_scanners = []
    for name, exts in scanner_filters():
        cexts = {e for e in exts if e in SOURCE_EXTS}
        if not cexts:
            continue
        missed = sorted(rel(p) for p in comp
                        if os.path.splitext(p)[1] not in cexts)
        seen_orphans = sorted(p for p in orph
                              if os.path.splitext(p)[1] in cexts)
        src_scanners.append((name, cexts, len(missed), len(seen_orphans)))
    width = max(len(n) for n, _, _, _ in src_scanners)
    print("  %-*s  %-22s %8s %8s" % (width, "tool", "accepts", "blind", "orphans"))
    for name, cexts, missed, seen in sorted(src_scanners):
        print("  %-*s  %-22s %8d %8d"
              % (width, name, ",".join(sorted(cexts)), missed, seen))
    print("\n  blind   = compiled sources the filter excludes")
    print("  orphans = uncompiled sources the filter admits")
    return 0


def self_test():
    ok = True

    def chk(label, cond):
        nonlocal ok
        ok = ok and bool(cond)
        print("  %-58s %s" % (label, "PASS" if cond else "FAIL"))

    comp, disk, orph = compiled_sources(), on_disk_sources(), orphan_sources()
    chk("the build compiles something at all", len(comp) > 500)
    chk("every compiled source exists on disk", not (comp - disk))
    chk("the two partitions cover the tree", comp | orph == disk)

    # positive control on the blind-spot direction: the build compiles units a
    # .c-only walk cannot reach, and their objects are real.
    non_c = sorted(p for p in comp if not p.endswith(".c"))
    chk("compiled non-.c units exist (blind-spot direction)", len(non_c) == 3)
    objs = [os.path.join(REPO, "build/GSAE01/src",
                         os.path.splitext(rel(p))[0][len("src/"):] + ".o")
            for p in non_c]
    chk("each of them really produces an object",
        all(os.path.exists(o) for o in objs))

    # positive control on the false-population direction, with the row that
    # produced a wrong verdict.
    init = os.path.join(REPO, "src/dolphin/os/__ppc_eabi_init.cpp")
    chk("__ppc_eabi_init.cpp is on disk", os.path.isfile(init))
    chk("__ppc_eabi_init.cpp is NOT compiled", init in orph)
    chk("its sibling .c IS compiled",
        os.path.join(REPO, "src/dolphin/os/__ppc_eabi_init.c") in comp)

    # negative control: the partition is not vacuous in either direction.
    chk("the orphan set is non-empty", len(orph) > 0)
    chk("no orphan sits in a game root",
        not [p for p in orph
             if rel(p).startswith(("src/main/", "src/track/", "src/dlls/"))])

    print("SELF-TEST", "PASS" if ok else "FAIL")
    return 0 if ok else 1


def main():
    args = sys.argv[1:]
    if args and args[0] in ("-h", "--help"):
        print(__doc__)
        return 0
    if args and args[0] == "--self-test":
        return self_test()
    if args and args[0] == "--orphans":
        for p in sorted(orphan_sources()):
            print(rel(p))
        return 0
    return report()


if __name__ == "__main__":
    sys.exit(main())
