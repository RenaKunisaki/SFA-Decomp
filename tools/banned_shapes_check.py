#!/usr/bin/env python3
"""Scan game code for the CLAUDE.md banned constructs, so the ban self-enforces.

Scope is GAME CODE ONLY -- src/main/, src/track/, src/dlls/. src/dolphin/ is the
SDK and is exempt by policy; it legitimately carries pragmas, gotos, __declspec
and lbl_ constants, so scanning it would produce nothing but false positives.

Eight pattern classes, each carrying its citation:

  PRAGMA          any #pragma in a game TU. Per-function pragma sandwiches of
                  every kind were purged repo-wide; pragmas may only be
                  configured at TU level via configure.py cflags.
  GOTO            write structured control flow instead.
  DECLSPEC_SECTION  __declspec(section ...) and any section-forcing placement.
  VOLATILE_PUN    volatile / *(volatile T*)& puns used to block CSE or hoisting.
                  volatile is legal ONLY for genuine hardware/interrupt
                  semantics, so a write through a hardware ADDRESS (the GX FIFO
                  at 0xCC008000) or a WGPipe-style pipe macro is allowed. The
                  banned shape takes the address of a C object: *(volatile T*)&x.
  VOLATILE_DECL   the DECLARATION form of the same hack: a file-scope volatile
                  object of FLOATING-POINT type. Genuine volatiles in this tree
                  are integer status flags written from a DVD/ARQ/retrace
                  callback, or a PPCWGPipe mapped at a hardware address; a
                  volatile f32 is a CSE/hoist blocker on ordinary data. Scoped to
                  file scope on purpose: an indented `volatile float y;` local is
                  the project's extern-inline-sqrtf return-slot idiom, which is
                  how sqrtf is written and is not a hack.
  REGISTER_ASM    dummy global-register reservations such as
                  register int unused asm("r14"), whose only purpose is to
                  remove a register from MWCC's allocator.
  LBL_CONST_DEF   pool-reconstruction consts: an lbl_8XXXXXXX-named SCALAR const
                  definition (or the const union { f32 f; } disguise) that exists
                  to force a pool symbol. Write the plain literal instead.
                  NOTE: an lbl_-named ARRAY or struct table is NOT this ban -- it
                  is real data whose name has not been recovered yet. Only scalar
                  definitions are flagged, and --strict-lbl additionally reports
                  arrays as naming debt (informational, never gating).
  SINGLE_ELEM_CONST_ARRAY
                  const T name[1] = {...} whose only reads are name[0] -- a
                  one-element array written to pin a pool slot. Two-pass: the
                  definition regex, then a reads census over the whole tree.
                  KEYED ON THE DEFINITION, because the definition is what makes
                  it the banned construct; the reads census only CLASSIFIES:
                  read only as name[0] -> pool anchor; NEVER READ -> anchor AND
                  dead (the purest form, and the one a reads-keyed check silently
                  misses); indexed or otherwise used -> a real array, not this
                  shape. ALLOWED EXCEPTION: a genuine cross-TU object -- the
                  symbol appears in config/GSAE01/symbols.txt or is referenced
                  from a different source file.

Exit status: 0 when there are no hits beyond the baseline, 1 otherwise, so this
can gate. --baseline rewrites the baseline file from the current tree; hits
beyond baseline are regrowth and fail the gate.

  python3 tools/banned_shapes_check.py              # gate
  python3 tools/banned_shapes_check.py --list       # every hit, ignore baseline
  python3 tools/banned_shapes_check.py --baseline   # record current as accepted
  python3 tools/banned_shapes_check.py --self-test  # validate BOTH directions

--self-test is not optional decoration. This tool gates future work, so it
validates against ground truth in both directions before anyone trusts a census
from it: it must FIRE on the historical hacks recorded at the pre-hack-purge tag
(a real positive corpus of thousands of instances) and must stay SILENT on
src/dolphin and on the genuine hardware-volatile sites. A checker that reports
zero is indistinguishable from a checker that is broken -- which is exactly how
a shell word-splitting bug once made this very tree look clean on every
pattern. Never trust a zero without the positive control.
"""

import argparse
import os
import re
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCAN_ROOTS = ["src/main", "src/track", "src/dlls"]
EXEMPT_ROOTS = ["src/dolphin"]
BASELINE = "tools/banned_shapes_baseline.txt"
SYMBOLS = "config/GSAE01/symbols.txt"

CITE = {
    "PRAGMA": "CLAUDE.md Banned constructs: any #pragma (TU-level cflags only)",
    "GOTO": "CLAUDE.md Banned constructs: goto -- write structured control flow",
    "DECLSPEC_SECTION": "CLAUDE.md Banned constructs: __declspec(section ...)",
    "VOLATILE_PUN": "CLAUDE.md Banned constructs: match-volatiles",
    "VOLATILE_DECL": "CLAUDE.md Banned constructs: match-volatiles (declaration form)",
    "REGISTER_ASM": "CLAUDE.md Banned constructs: dummy global-register reservations",
    "LBL_CONST_DEF": "CLAUDE.md Banned constructs: pool-reconstruction consts",
    "SINGLE_ELEM_CONST_ARRAY": "CLAUDE.md Banned constructs: pool-reconstruction consts (1-element pin)",
    "LBL_ARRAY_NAMING_DEBT": "informational: unrecovered name, NOT a ban",
}

RE_PRAGMA = re.compile(r"^\s*#\s*pragma\b")
RE_GOTO = re.compile(r"(?<![\w.>])goto\s+[A-Za-z_]\w*\s*;")
RE_DECLSPEC = re.compile(r"__declspec\s*\(\s*section\b")
RE_VOLATILE_CAST = re.compile(r"\*\s*\(\s*volatile\b")
RE_REGISTER_ASM = re.compile(r"\bregister\b[^;]*\basm\s*\(")
# file-scope (column 0) volatile object of floating-point type
RE_VOLATILE_DECL = re.compile(
    r"^(?:extern\s+)?volatile\s+(?:const\s+)?(?:f32|f64|float|double)\b")
# scalar lbl_ const definition: no '[' before '=' -> not an array/table
RE_LBL_SCALAR = re.compile(
    r"^\s*(?:static\s+)?const\s+(?!union\b)[A-Za-z_]\w*\s*\*?\s*"
    r"(lbl_[0-9A-Fa-f]{8})\s*=")
RE_LBL_UNION = re.compile(r"^\s*(?:static\s+)?const\s+union\b[^;]*?(lbl_[0-9A-Fa-f]{8})\s*=")
RE_LBL_ARRAY = re.compile(
    r"^\s*(?:static\s+)?const\s+[A-Za-z_]\w*\s*\*?\s*(lbl_[0-9A-Fa-f]{8})\s*\[")
RE_ONE_ELEM = re.compile(
    r"^\s*(?:static\s+)?const\s+[A-Za-z_]\w*\s*\*?\s*([A-Za-z_]\w*)\s*\[\s*1\s*\]\s*=")

# A volatile cast is GENUINE hardware when it targets a hardware address or a
# write-gather pipe, rather than the address of a C object.
RE_HW_ADDR = re.compile(r"0x[Cc][Cc][0-9A-Fa-f]{5}")
RE_PIPE = re.compile(r"WG(?:Pipe|Fifo)", re.I)
RE_ADDR_OF_OBJECT = re.compile(r"\*\s*\(\s*volatile\b[^)]*\)\s*&")


def is_c_source(path):
    return path.endswith(".c") or path.endswith(".h")


def walk(roots, base=REPO):
    for root in roots:
        for dirpath, _dirs, files in os.walk(os.path.join(base, root)):
            for f in sorted(files):
                if is_c_source(f):
                    p = os.path.join(dirpath, f)
                    yield os.path.relpath(p, base)


def strip_line_comment(line):
    i = line.find("//")
    return line[:i] if i >= 0 else line


def scan_file(rel, text, strict_lbl=False):
    """Return single-line hits as (rel, lineno, cls, snippet)."""
    hits = []
    for n, raw in enumerate(text.splitlines(), 1):
        line = strip_line_comment(raw)
        if not line.strip():
            continue
        if RE_PRAGMA.search(line):
            hits.append((rel, n, "PRAGMA", raw.strip()))
        if RE_GOTO.search(line):
            hits.append((rel, n, "GOTO", raw.strip()))
        if RE_DECLSPEC.search(line):
            hits.append((rel, n, "DECLSPEC_SECTION", raw.strip()))
        if RE_REGISTER_ASM.search(line):
            hits.append((rel, n, "REGISTER_ASM", raw.strip()))
        if RE_VOLATILE_DECL.match(line) and not (
                RE_HW_ADDR.search(line) or RE_PIPE.search(line)):
            hits.append((rel, n, "VOLATILE_DECL", raw.strip()))
        if RE_VOLATILE_CAST.search(line):
            hardware = RE_HW_ADDR.search(line) or RE_PIPE.search(line)
            pun = RE_ADDR_OF_OBJECT.search(line)
            if pun or not hardware:
                hits.append((rel, n, "VOLATILE_PUN", raw.strip()))
        m = RE_LBL_SCALAR.match(line) or RE_LBL_UNION.match(line)
        if m:
            hits.append((rel, n, "LBL_CONST_DEF", raw.strip()))
        elif strict_lbl and RE_LBL_ARRAY.match(line):
            hits.append((rel, n, "LBL_ARRAY_NAMING_DEBT", raw.strip()))
    return hits


def load_symbols(base=REPO):
    p = os.path.join(base, SYMBOLS)
    if not os.path.isfile(p):
        return set()
    names = set()
    with open(p, errors="ignore") as fh:
        for line in fh:
            m = re.match(r"\s*([A-Za-z_]\w*)\s*=", line)
            if m:
                names.add(m.group(1))
    return names


def scan_one_elem(files, base=REPO):
    """Two-pass check for `const T name[1] = {...}` pinned pool slots.

    Pass 1 finds definitions. Pass 2 censuses reads across the whole tree, so a
    symbol used as a real array (any subscript other than [0], or its bare name
    passed along) is not flagged. Cross-TU symbols are the allowed exception.
    """
    defs = []
    for rel, text in files.items():
        for n, raw in enumerate(text.splitlines(), 1):
            line = strip_line_comment(raw)
            m = RE_ONE_ELEM.match(line)
            if m:
                defs.append((rel, n, m.group(1), raw.strip()))
    if not defs:
        return []
    symbols = load_symbols(base)
    hits = []
    for rel, n, name, snippet in defs:
        zero_reads = other_reads = foreign = 0
        for orel, text in files.items():
            if name not in text:
                continue
            for idx, raw in enumerate(text.splitlines(), 1):
                if orel == rel and idx == n:
                    continue          # the definition itself
                line = strip_line_comment(raw)
                for mm in re.finditer(r"\b" + re.escape(name) + r"\b", line):
                    rest = line[mm.end():]
                    if re.match(r"\s*\[\s*0\s*\]", rest):
                        zero_reads += 1
                    else:
                        other_reads += 1
                    if orel != rel:
                        foreign += 1
        # The DEFINITION shape is what makes this the banned construct; the reads
        # census only CLASSIFIES it. Keying on reads instead silently missed the
        # purest case -- a definition that is never read at all (its only purpose
        # is to occupy a pool slot).
        if (name in symbols) or foreign:
            continue                                   # allowed cross-TU exception
        if other_reads:
            continue                                   # used as a real array
        note = " [never read -- dead pool anchor]" if not zero_reads else ""
        hits.append((rel, n, "SINGLE_ELEM_CONST_ARRAY", snippet + note))
    return hits


def collect(roots=SCAN_ROOTS, base=REPO, strict_lbl=False):
    files = {}
    for rel in walk(roots, base):
        try:
            with open(os.path.join(base, rel), errors="ignore") as fh:
                files[rel] = fh.read()
        except OSError:
            continue
    hits = []
    for rel, text in sorted(files.items()):
        hits += scan_file(rel, text, strict_lbl)
    hits += scan_one_elem(files, base)
    return sorted(hits, key=lambda h: (h[0], h[1], h[2]))


def key(h):
    return "%s:%d:%s" % (h[0], h[1], h[2])


def read_baseline():
    p = os.path.join(REPO, BASELINE)
    if not os.path.isfile(p):
        return set()
    out = set()
    with open(p) as fh:
        for line in fh:
            line = line.strip()
            if line and not line.startswith("#"):
                out.add(line)
    return out


def write_baseline(hits):
    p = os.path.join(REPO, BASELINE)
    with open(p, "w") as fh:
        fh.write("# tools/banned_shapes_check.py baseline -- accepted existing instances.\n")
        fh.write("# Anything NOT listed here is regrowth and fails the gate.\n")
        fh.write("# Shrink this file; never grow it.\n")
        for h in hits:
            fh.write(key(h) + "\n")
    return p


def report(hits, title):
    print("%s: %d" % (title, len(hits)))
    by = {}
    for h in hits:
        by.setdefault(h[2], []).append(h)
    for cls in sorted(by):
        print("  %-24s %4d   [%s]" % (cls, len(by[cls]), CITE.get(cls, "")))
        for rel, n, _c, snip in by[cls]:
            print("      %s:%d: %s" % (rel, n, snip[:100]))


def self_test():
    """Validate in BOTH directions before this tool is allowed to gate."""
    ok = True

    def chk(label, cond, detail=""):
        nonlocal ok
        if not cond:
            ok = False
        print("  %-56s %s %s" % (label, "PASS" if cond else "*** FAIL ***", detail))

    # NEGATIVE 1: the SDK is out of scope and must never appear in results.
    hits = collect()
    chk("no result is inside an exempt root",
        not any(h[0].startswith(tuple(EXEMPT_ROOTS)) for h in hits))

    # NEGATIVE 2: genuine hardware volatiles are not puns.
    hw = [h for h in hits if h[2] == "VOLATILE_PUN" and RE_HW_ADDR.search(h[3])
          and "&" not in h[3]]
    chk("genuine GX FIFO volatiles not flagged", not hw,
        "" if not hw else "leaked %d" % len(hw))

    # NEGATIVE 3: genuine interrupt/status volatiles (integer flags written from a
    # DVD/ARQ/retrace callback) and hardware pipes must never be flagged. This is
    # the reconciliation guard for the 48-vs-49 delta: the rule that catches
    # gCloudActionGlareQuadSize must not catch these.
    vd = [h for h in hits if h[2] == "VOLATILE_DECL"]
    leaked = [h for h in vd if re.search(r"\b(?:int|u8|u16|u32|s8|s16|s32|BOOL)\b", h[3])
              or RE_PIPE.search(h[3])]
    chk("interrupt/status + pipe volatiles not flagged", not leaked,
        "" if not leaked else "leaked %d" % len(leaked))

    # NEGATIVE 4: the extern-inline-sqrtf return slot is an indented LOCAL and is
    # not a file-scope declaration; it must not be flagged.
    chk("sqrtf volatile return-slot local not flagged",
        not [h for h in vd if h[3].startswith(("volatile float y", "volatile f32 root"))
             and h[3] != h[3].lstrip()])
    chk("VOLATILE_DECL pattern fires on a float file-scope volatile",
        bool(RE_VOLATILE_DECL.match("volatile f32 gX[2] = {1.0f, 0.0f};")))
    chk("VOLATILE_DECL pattern ignores an int file-scope volatile",
        not RE_VOLATILE_DECL.match("volatile int gFlag;"))

    # REGRESSION GUARD for the 48-vs-49 reconciliation: two independent scans
    # disagreed by one, and the delta was a one-element const array that is NEVER
    # READ. A reads-keyed check cannot see it, so the class is keyed on the
    # definition. This asserts the never-read variant stays caught.
    oe = [h for h in hits if h[2] == "SINGLE_ELEM_CONST_ARRAY"]
    chk("never-read one-element const array is caught",
        any("gWarpPadRestZero" in h[3] for h in oe))
    chk("never-read variant is labelled as dead",
        any("gWarpPadRestZero" in h[3] and "never read" in h[3] for h in oe))

    # POSITIVE: the SDK really does contain the shapes, so the patterns fire
    # when pointed at code that has them. This is the control that proves a
    # zero on game code means "clean", not "broken".
    sdk = collect(roots=EXEMPT_ROOTS)
    got = {h[2] for h in sdk}
    for cls in ("PRAGMA", "GOTO", "DECLSPEC_SECTION"):
        chk("pattern fires on SDK corpus: %s" % cls, cls in got)

    # POSITIVE: the historical purge corpus at pre-hack-purge.
    try:
        tag = subprocess.run(["git", "-C", REPO, "rev-parse", "-q", "--verify",
                              "pre-hack-purge^{commit}"],
                             capture_output=True, text=True)
        if tag.returncode == 0:
            out = subprocess.run(
                ["git", "-C", REPO, "grep", "-cE", r"^\s*#\s*pragma",
                 "pre-hack-purge", "--", "src/main/*.c"],
                capture_output=True, text=True).stdout
            total = sum(int(l.rsplit(":", 1)[1]) for l in out.splitlines() if ":" in l)
            chk("historical corpus has pragmas to catch", total > 100, "(%d)" % total)
        else:
            chk("pre-hack-purge tag present", False, "(tag missing -- corpus unavailable)")
    except Exception as exc:                                   # pragma: no cover
        chk("historical corpus reachable", False, str(exc))

    # SANITY: scanning nothing must yield nothing (guards the walker).
    chk("empty scope yields empty result", collect(roots=["src/does_not_exist"]) == [])
    return ok


def main():
    ap = argparse.ArgumentParser(description="Gate game code against CLAUDE.md banned constructs.")
    ap.add_argument("--list", action="store_true", help="print every hit, ignoring the baseline")
    ap.add_argument("--baseline", action="store_true", help="rewrite the baseline from the current tree")
    ap.add_argument("--self-test", action="store_true", help="validate the checker in both directions")
    ap.add_argument("--strict-lbl", action="store_true",
                    help="also report lbl_-named arrays as naming debt (informational)")
    args = ap.parse_args()

    if args.self_test:
        print("banned_shapes_check self-test")
        ok = self_test()
        print("\nSELF-TEST %s" % ("PASSED" if ok else "FAILED"))
        return 0 if ok else 1

    hits = collect(strict_lbl=args.strict_lbl)
    informational = [h for h in hits if h[2] == "LBL_ARRAY_NAMING_DEBT"]
    hits = [h for h in hits if h[2] != "LBL_ARRAY_NAMING_DEBT"]

    if args.baseline:
        p = write_baseline(hits)
        print("baseline written: %s (%d accepted instances)" % (p, len(hits)))
        report(hits, "recorded")
        return 0

    if args.list:
        report(hits, "all hits")
        if informational:
            report(informational, "informational (never gating)")
        return 0

    base = read_baseline()
    new = [h for h in hits if key(h) not in base]
    fixed = base - {key(h) for h in hits}
    print("scanned %d files under %s" % (
        sum(1 for _ in walk(SCAN_ROOTS)), ", ".join(SCAN_ROOTS)))
    print("hits=%d  baseline=%d  regrowth=%d  fixed-since-baseline=%d"
          % (len(hits), len(base), len(new), len(fixed)))
    if fixed:
        print("\n%d baseline entries no longer present -- shrink the baseline:" % len(fixed))
        for k in sorted(fixed):
            print("      %s" % k)
    if new:
        print()
        report(new, "REGROWTH (not in baseline)")
        print("\nThese are banned in game code. See CLAUDE.md 'Banned constructs' "
              "and docs/HACK_AUDIT.md.")
        return 1
    print("\nOK -- no banned-shape regrowth.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
