#!/usr/bin/env python3
"""Mutation controls for this project's checkers: prove each one CAN fail.

Why this exists
---------------
Every screen in tools/ earns its keep by reporting a ZERO.  A zero is only
evidence if the instrument was capable of reporting something else, and that is
not something a passing run can tell you.  The campaign has now produced two
demonstrations that it is a real hazard, not a hypothetical:

  * a parse-soundness check imported the parser's own precedence table, so on
    the one question it existed to ask it agreed by construction.  Its
    re-association mutation came back 57258/57258 CLEAN; with a locally written
    table the same run found 2144 mis-parses.

  * `tools/semantic_equivalence.py` decided operand purity with
    `cexpr.is_pure`, and `tools/expr_sweep.py` calls the SAME predicate to
    decide whether to generate the rewrite in the first place.  Generator and
    prover therefore agreed by construction, and the differential evaluator
    could not back them up because it evaluates each spelling with its own
    fresh Evaluator, so a discarded `i++` reads identically on both sides.
    Forcing `is_pure` to True blessed `f(x) + y -> y + f(x)`.

So: deliberately corrupt the SUBJECT (or the model), and require the instrument
to scream.  Three mutation families.

  SCOPE     run the screen with a unit filter that matches nothing.  A screen
            that answers `scanned=0 ... =0` and exits 0 will answer the same
            way for a typo'd unit name, and the operator reads it as clean.
            The instrument must REFUSE an empty scope, not report it clean.

  OBJECT    hand the screen a deliberately damaged copy of a real object --
            a section resized, its alignment changed, a byte flipped in .text,
            a symbol renamed, two symbols' addresses swapped -- through a
            shadow repo root, and require a finding.  This is the only control
            that proves the two-sided comparators are wired to their subject at
            all.  The ELF patcher here is written from the spec with `struct`
            and shares no code with any instrument it tests.

  DELEGATE  the instrument owns its own mutation suite; run it.

Usage:
  python3 tools/vacuity_audit.py                # every family
  python3 tools/vacuity_audit.py --family scope
  python3 tools/vacuity_audit.py --list
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PY = sys.executable


# ------------------------------------------------------------------ ELF32 BE
#
# Written from the ELF spec so the mutation shares nothing with the readers it
# is used to test.  Only the fields the mutations need.

class Elf:
    def __init__(self, data: bytes):
        self.b = bytearray(data)
        if self.b[:4] != b"\x7fELF" or self.b[5] != 2:
            raise ValueError("not a big-endian ELF")
        self.shoff = struct.unpack_from(">I", self.b, 0x20)[0]
        self.shentsize = struct.unpack_from(">H", self.b, 0x2E)[0]
        self.shnum = struct.unpack_from(">H", self.b, 0x30)[0]
        self.shstrndx = struct.unpack_from(">H", self.b, 0x32)[0]

    def sh(self, i):
        o = self.shoff + i * self.shentsize
        (name, typ, flags, addr, off, size, link, info,
         align, entsize) = struct.unpack_from(">10I", self.b, o)
        return dict(i=i, o=o, name=name, type=typ, flags=flags, addr=addr,
                    off=off, size=size, link=link, info=info, align=align,
                    entsize=entsize)

    def _str(self, strtab_off, idx):
        e = self.b.index(b"\0", strtab_off + idx)
        return self.b[strtab_off + idx:e].decode("latin-1")

    def secname(self, s):
        return self._str(self.sh(self.shstrndx)["off"], s["name"])

    def find(self, want):
        for i in range(self.shnum):
            s = self.sh(i)
            if self.secname(s) == want:
                return s
        return None

    def set_field(self, s, field, value):
        idx = ["name", "type", "flags", "addr", "off", "size", "link",
               "info", "align", "entsize"].index(field)
        struct.pack_into(">I", self.b, s["o"] + idx * 4, value)

    def symtab(self):
        s = self.find(".symtab")
        if s is None:
            return None, None
        return s, self.sh(s["link"])


def mutate_object(path: str, kind: str) -> str:
    """Damage a copy of `path` in place.  Returns a human description."""
    e = Elf(open(path, "rb").read())
    if kind == "size":
        t = e.find(".text") or e.find(".data")
        e.set_field(t, "size", max(0, t["size"] - 4))
        what = "%s size %d -> %d" % (e.secname(t), t["size"], t["size"] - 4)
    elif kind == "align":
        t = e.find(".text") or e.find(".data")
        e.set_field(t, "align", 64 if t["align"] != 64 else 128)
        what = "%s alignment -> 64" % e.secname(t)
    elif kind == "content":
        t = e.find(".text")
        if t is None or t["size"] < 8:
            raise ValueError("no .text to corrupt")
        e.b[t["off"] + 4] ^= 0x01
        what = ".text byte at +4 flipped"
    elif kind in ("symname", "symorder"):
        st, strt = e.symtab()
        if st is None:
            raise ValueError("no .symtab")
        n = st["size"] // 16
        funcs = []
        for k in range(n):
            o = st["off"] + k * 16
            nameoff, value, sz, info, other, shndx = struct.unpack_from(
                ">IIIBBH", e.b, o)
            if (info & 0xF) == 2 and sz:                    # STT_FUNC
                funcs.append((o, nameoff, value, sz))
        if len(funcs) < 2:
            raise ValueError("fewer than two functions")
        if kind == "symname":
            o, nameoff, _v, _s = funcs[0]
            base = strt["off"] + nameoff
            end = e.b.index(b"\0", base)
            e.b[end - 1] = ord("Z") if e.b[end - 1] != ord("Z") else ord("Y")
            what = "renamed the first function's last character"
        else:
            (o1, _n1, v1, _s1), (o2, _n2, v2, _s2) = funcs[0], funcs[1]
            struct.pack_into(">I", e.b, o1 + 4, v2)
            struct.pack_into(">I", e.b, o2 + 4, v1)
            what = "swapped two function addresses"
    else:
        raise ValueError("unknown mutation " + kind)
    open(path, "wb").write(bytes(e.b))
    return what


# --------------------------------------------------------------- shadow root

def shadow_root(unit_name: str, mutation: str, tmp: str, tag: str = None):
    """A complete miniature repo root holding ONE unit.

    Screens find their root two different ways -- some from
    `os.path.abspath(__file__)`, some from `Path(__file__).resolve()` (which
    follows a symlink straight back to the real tree), and some by directory
    convention under `build/<ver>/{src,obj}` rather than through a manifest.
    So the shadow COPIES tools/ and lays out a real `build/GSAE01/{src,obj}`
    pair plus both manifests: whichever route a screen takes, it lands here.

    `mutation` of None builds the shadow undamaged -- that is the control run
    the mutated run must differ from, without which "it reported a finding"
    proves nothing about whether it saw the mutation.
    """
    units = json.load(open(os.path.join(REPO, "objdiff.json")))["units"]
    u = next((x for x in units if x["name"] == unit_name), None)
    if u is None or not u.get("base_path") or not u.get("target_path"):
        return None, "unit %s has no two-sided objects" % unit_name
    ours = os.path.join(REPO, u["base_path"])
    retail = os.path.join(REPO, u["target_path"])
    if not (os.path.exists(ours) and os.path.exists(retail)):
        return None, "objects for %s are not built" % unit_name

    root = os.path.join(tmp, tag or (mutation or "clean"))
    os.makedirs(root, exist_ok=True)
    shutil.copytree(os.path.join(REPO, "tools"), os.path.join(root, "tools"),
                    ignore=shutil.ignore_patterns("__pycache__", "refcorpus",
                                                  "xref", "dolphin_mcp",
                                                  "ghidra_scripts"))
    rel_src = u["base_path"]
    rel_obj = u["target_path"]
    for rel, src in ((rel_src, ours), (rel_obj, retail)):
        dst = os.path.join(root, rel)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        shutil.copyfile(src, dst)
    os.symlink(os.path.join(REPO, "build", "binutils"),
               os.path.join(root, "build", "binutils"))
    os.symlink(os.path.join(REPO, "build", "tools"),
               os.path.join(root, "build", "tools"))
    for extra in ("config", "src", "include"):
        p = os.path.join(REPO, extra)
        if os.path.exists(p):
            os.symlink(p, os.path.join(root, extra))

    what = "undamaged control"
    if mutation:
        what = mutate_object(os.path.join(root, rel_src), mutation)

    v = dict(u)
    v["base_path"] = rel_src
    v["target_path"] = rel_obj
    json.dump({"units": [v]}, open(os.path.join(root, "objdiff.json"), "w"))
    cfg_p = os.path.join(REPO, "build", "GSAE01", "config.json")
    if os.path.exists(cfg_p):
        cfg = json.load(open(cfg_p))
        cfg["units"] = [x for x in cfg["units"]
                        if x.get("object") == rel_obj]
        json.dump(cfg, open(os.path.join(root, "build", "GSAE01",
                                         "config.json"), "w"))
    return root, what


# ------------------------------------------------------------------ registry
#
# `screams` is a predicate over (returncode, text).  The DEFAULT is "the tool
# did not answer a bare zero"; a tool that must gate additionally has to exit
# non-zero.

def _refuses_empty(rc, t):
    """An empty scope must be an ERROR, not a clean zero."""
    if rc != 0:
        return True
    return bool(re.search(r"no units? match|nothing to (?:scan|check)|"
                          r"EMPTY SCOPE|refus", t, re.I))


def _has_finding(rc, t):
    return rc != 0 or bool(re.search(
        r"MISMATCH|DIFFER|ONLY-OURS|RETAIL-ONLY|misordered=[1-9]|"
        r"real-mismatch=[1-9]|content-mismatch=[1-9]|first divergence|"
        r"CONTENT|RELOC|ours (?:SHORT|LARGER)", t))


# name -> (argv-from-root, object mutations it must detect)
OBJECT_SCREENS = {
    "section_size_check": ("size",),
    "section_align_check": ("align",),
    "order_check": ("symorder",),
    "pairing_check": ("symname",),
    "obj_equal": ("content", "size"),
}

# Screens that accept a unit filter.  An empty scope must be refused.
SCOPE_SCREENS = [
    "section_size_check", "order_check", "pairing_check", "pool_content_check",
    "reloc_width_check", "skew_value_check", "f64_half_check",
    "data_reloc_check", "size_parity_check", "section_align_check",
    "data_sym_size_check", "sda_base_register_check", "unit_bytes_check",
]

# Instruments that own their mutation suite.
DELEGATED = [
    ("semantic_equivalence", ["--mutate"]),
    ("cexpr_roundtrip", ["--self-test"]),
    ("cexpr", ["--self-test"]),
    ("banned_shapes_check", ["--self-test"]),
    ("obj_equal", ["--self-test"]),
    ("stmt_sweep", ["--self-test"]),
    ("stmt_sweep", ["--self-test", "--mutate"]),
    ("score_delta_gate", ["--self-test"]),
    ("missing_param_check", ["--selftest"]),
]


def run(argv, cwd=None, timeout=900):
    try:
        r = subprocess.run(argv, cwd=cwd or REPO, capture_output=True,
                           text=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return 124, "TIMEOUT"
    except Exception as exc:                                  # pragma: no cover
        return 125, str(exc)
    return r.returncode, (r.stdout or "") + (r.stderr or "")


def family_scope(verbose):
    print("=== MUTATION: EMPTY SCOPE "
          "(a filter that matches nothing must be refused, not reported clean) ===")
    print("    each row first PROVES the argument is a filter at all, by "
          "requiring the\n    filtered and unfiltered runs to differ -- "
          "otherwise the probe itself is vacuous.")
    bad = 0
    for tool in SCOPE_SCREENS:
        p = os.path.join(REPO, "tools", tool + ".py")
        if not os.path.exists(p):
            continue
        rc0, full = run([PY, p])
        token = "__vacuity_no_such_unit__"
        rc, out = run([PY, p, token])
        # Some screens spell their positional as an OUTPUT PATH, not a filter.
        spilled = os.path.join(REPO, token)
        if os.path.exists(spilled):
            os.unlink(spilled)
            print("  [n/a ] %-24s the positional argument is an output path"
                  % tool)
            continue
        if out == full:
            print("  [n/a ] %-24s the positional argument is not a unit filter"
                  % tool)
            continue
        ok = _refuses_empty(rc, out)
        bad += not ok
        print("  [%s] %-24s rc=%-3d %s"
              % ("held" if ok else "VACUOUS", tool, rc,
                 " ".join(out.split())[:74]))
    return bad


def family_object(unit, verbose):
    print("\n=== MUTATION: DAMAGED OBJECT "
          "(a real object, deliberately broken, must produce a finding) ===")
    print("    every row is run TWICE -- once against an UNDAMAGED shadow of the "
          "same unit --\n    and the two outputs must DIFFER, or the finding was "
          "not caused by the mutation.")
    bad = 0
    tmp = tempfile.mkdtemp(prefix="vacuity_")
    try:
        base_root, _ = shadow_root(unit, None, tmp, tag="control")
        if base_root is None:
            print("  [skip] no two-sided objects for %s" % unit)
            return 0
        for tool, kinds in sorted(OBJECT_SCREENS.items()):
            for kind in kinds:
                root, what = shadow_root(unit, kind, tmp,
                                         tag="%s_%s" % (tool, kind))
                if root is None:
                    print("  [skip] %-24s %s" % (tool, what))
                    continue

                def go(r):
                    if tool == "obj_equal":
                        return run([PY, os.path.join(r, "tools",
                                                     "obj_equal.py"),
                                    os.path.join(r, "build", "GSAE01", "src",
                                                 *unit.split("/")[1:]) + ".o",
                                    os.path.join(r, "build", "GSAE01", "obj",
                                                 *unit.split("/")[1:]) + ".o"],
                                   cwd=r)
                    return run([PY, os.path.join(r, "tools", tool + ".py")],
                               cwd=r)

                rc, out = go(root)
                rc0, ctl = go(base_root)
                saw = out != ctl
                ok = _has_finding(rc, out) and saw
                bad += not ok
                note = "" if saw else "  <-- IDENTICAL TO THE CONTROL"
                print("  [%s] %-22s %-9s %s -> %s%s"
                      % ("held" if ok else "BLIND", tool, kind, what,
                         " ".join(out.split())[:44], note))
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    return bad


def family_delegate(verbose):
    print("\n=== DELEGATED: instruments that own a mutation suite ===")
    bad = 0
    for tool, args in DELEGATED:
        p = os.path.join(REPO, "tools", tool + ".py")
        if not os.path.exists(p):
            continue
        rc, out = run([PY, p] + args)
        ok = rc == 0
        bad += not ok
        tail = [l for l in out.strip().splitlines() if l.strip()][-1:] or [""]
        print("  [%s] %-24s %-12s %s"
              % ("held" if ok else "FAILED", tool, " ".join(args),
                 tail[0].strip()[:64]))
    return bad


def family_build(unit_object, verbose):
    """The two instruments every declaration sweep silently depends on.

    A sweep ranks orderings on `unitfuzzy.measure` and compiles them with
    `direct_build`, and NEITHER is what the standing gate uses.  If unitfuzzy's
    number diverged from report.json, or if direct_build's object were not
    byte-for-byte ninja's, every sweep in this campaign would have optimised
    something the gate cannot see -- and both would still have looked perfectly
    healthy from inside.

    NOTE: this family WRITES into build/<ver>/src.  Do not run it against a
    worktree that has a sweep in flight; point --unit-object at a unit no
    sweep touches.
    """
    print("\n=== BUILD/MEASURE CHAIN "
          "(what the sweeps actually rank and compile on) ===")
    bad = 0
    sys.path.insert(0, os.path.join(REPO, "tools"))
    import unitfuzzy                                          # noqa: E402
    from direct_build import direct_build                     # noqa: E402
    import hashlib

    def md5(p):
        return hashlib.md5(open(p, "rb").read()).hexdigest()

    src_o = os.path.join(REPO, unit_object.replace("/obj/", "/src/"))
    if not os.path.exists(src_o):
        print("  [skip] %s is not built" % src_o)
        return 0
    before = md5(src_o)
    os.unlink(src_o)
    ok_build = direct_build(unit_object, "GSAE01")
    after = md5(src_o) if os.path.exists(src_o) else None
    ok = bool(ok_build) and after == before
    bad += not ok
    print("  [%s] direct_build reproduces ninja's object bytes  %s"
          % ("held" if ok else "DIVERGES", before[:12]))
    # non-vacuity: the comparison must be able to see a difference at all
    with open(src_o, "r+b") as fh:
        fh.seek(0x30)
        b = fh.read(1)
        fh.seek(0x30)
        fh.write(bytes([b[0] ^ 0xFF]))
    saw = md5(src_o) != before
    bad += not saw
    print("  [%s] ...and the md5 comparison can see a one-byte change"
          % ("held" if saw else "BLIND"))
    direct_build(unit_object, "GSAE01")

    rep = json.load(open(os.path.join(REPO, "build", "GSAE01",
                                      "report.json")))
    cfg = json.load(open(os.path.join(REPO, "build", "GSAE01",
                                      "config.json")))
    rmap = {}
    for u in rep["units"]:
        for f in u.get("functions") or []:
            rmap[(u["name"], f["name"])] = f.get("fuzzy_match_percent")
    checked = agreed = discriminating = 0
    seen = set()
    for (un, fn), v in rmap.items():
        if v is None or v >= 100.0 or un in seen:
            continue
        seen.add(un)
        base = "/".join(un.split("/")[1:]) + ".c"
        u = next((x for x in cfg["units"] if x["name"] == base), None)
        if u is None:
            continue
        try:
            m = unitfuzzy.measure(u, "GSAE01")
        except Exception:
            continue
        got = next((f["fuzzy_match_percent"]
                    for f in (m.get("functions") or []) if f["name"] == fn),
                   None)
        checked += 1
        agreed += got is not None and abs(got - v) < 1e-9
        discriminating += 1
        if checked >= 6:
            break
    ok = checked and agreed == checked and discriminating
    bad += not ok
    print("  [%s] unitfuzzy agrees with report.json on %d/%d sub-100 "
          "functions" % ("held" if ok else "DIVERGES", agreed, checked))
    return bad


def family_report(verbose):
    """Vacuities in the SCORE ITSELF -- reported, because they cannot be fixed
    from here, and a number nobody can fix is exactly the one people forget is
    not evidence."""
    print("\n=== THE REPORT AS AN INSTRUMENT ===")
    rep = json.load(open(os.path.join(REPO, "build", "GSAE01",
                                      "report.json")))
    cfg = json.load(open(os.path.join(REPO, "objdiff.json")))
    have = {u["name"]: bool(u.get("base_path")) for u in cfg["units"]}
    blind = [u for u in rep["units"] if not have.get(u["name"], True)]
    at100 = [u for u in blind
             if u["measures"].get("fuzzy_match_percent") == 100.0]
    print("  units in the report: %d;  with NO ours-side object: %d, of which "
          "%d read 100.0" % (len(rep["units"]), len(blind), len(at100)))
    print("     -> those %d are compared against THEMSELVES.  Their 100.0 is a "
          "tautology,\n        and they are inside complete_units/total_units."
          % len(at100))
    zero = sum(1 for u in rep["units"]
               for f in (u.get("functions") or [])
               if f.get("fuzzy_match_percent") is None)
    print("  functions carrying NO fuzzy_match_percent at all: %d "
          "(objdiff omits the field at 0.0, so a MISSING score must be read "
          "as -1, never as absent)" % zero)
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--family",
                    choices=("scope", "object", "delegate", "build",
                             "report"),
                    action="append")
    ap.add_argument("--unit-object",
                    default="build/GSAE01/obj/main/camera.o",
                    help="unit the BUILD family rebuilds "
                         "(must not be under a live sweep)")
    ap.add_argument("--unit", default="main/main/render",
                    help="unit whose objects the OBJECT family damages")
    ap.add_argument("--list", action="store_true")
    ap.add_argument("-v", "--verbose", action="store_true")
    a = ap.parse_args()
    if a.list:
        print("scope:   " + " ".join(SCOPE_SCREENS))
        print("object:  " + " ".join(sorted(OBJECT_SCREENS)))
        print("delegate:" + " ".join(t for t, _ in DELEGATED))
        return 0
    fams = a.family or ["scope", "object", "delegate", "report"]
    bad = 0
    if "scope" in fams:
        bad += family_scope(a.verbose)
    if "object" in fams:
        bad += family_object(a.unit, a.verbose)
    if "delegate" in fams:
        bad += family_delegate(a.verbose)
    if "build" in fams:
        bad += family_build(a.unit_object, a.verbose)
    if "report" in fams:
        bad += family_report(a.verbose)
    print("\nvacuity audit: %s" % ("ALL INSTRUMENTS SCREAMED" if not bad
                                   else "%d INSTRUMENT(S) STAYED SILENT" % bad))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
