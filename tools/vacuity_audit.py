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
to scream.  Six families; `scope`, `object`, `delegate` and `report` run by
default, `build` and `oracle` are opt-in because they write into the tree.

  SCOPE     run the screen with a unit filter that matches nothing.  A screen
            that answers `scanned=0 ... =0` and exits 0 will answer the same
            way for a typo'd unit name, and the operator reads it as clean.
            The instrument must REFUSE an empty scope, not report it clean.
            Each row first PROVES the argument is a filter at all -- the
            filtered and unfiltered runs must differ -- or the probe would be
            as vacuous as the thing it audits.

  OBJECT    hand the screen a deliberately damaged copy of a real object --
            a section resized, its alignment changed, a byte flipped in .text,
            a symbol renamed, a symbol's size changed, two symbols' addresses
            swapped -- through a shadow repo root, and require a finding.  This
            is the only control that proves the two-sided comparators are wired
            to their subject at all.  Every row also runs against an UNDAMAGED
            shadow and the outputs must differ, so a finding that was already
            there cannot pass for a detection.  The ELF patcher here is written
            from the spec with `struct` and shares no code with any instrument
            it tests.

  DELEGATE  the instrument owns its own mutation suite; run it.

  BUILD     what every declaration sweep silently rests on and nothing had ever
            asserted: `direct_build` must reproduce ninja's object BYTES, and
            `unitfuzzy` must agree with report.json.  (opt-in: writes objects)

  REPORT    vacuities in the score itself, reported rather than fixed.

  ORACLE    a POSITIVE control for tools/slot_oracle.py, whose census answers
            "pi not realisable within this block" almost every time -- exactly
            the shape a broken instrument produces.  (opt-in: writes source)

Usage:
  python3 tools/vacuity_audit.py                # scope+object+delegate+report
  python3 tools/vacuity_audit.py --family scope
  python3 tools/vacuity_audit.py --family build --family oracle
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
    elif kind == "symsize":
        st, _strt = e.symtab()
        if st is None:
            raise ValueError("no .symtab")
        for k in range(st["size"] // 16):
            o = st["off"] + k * 16
            _n, _v, sz, info, _o2, _sh = struct.unpack_from(">IIIBBH", e.b, o)
            if (info & 0xF) == 2 and sz > 8:                # STT_FUNC
                struct.pack_into(">I", e.b, o + 8, sz - 4)
                what = "first function's st_size %d -> %d" % (sz, sz - 4)
                break
        else:
            raise ValueError("no sized function")
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
    # a SECTION-size change is invisible to size_parity_check by design -- it
    # reads per-function symbol sizes -- so its control has to mutate the axis
    # it actually looks at, or the row would pass for the wrong reason.
    "size_parity_check": ("symsize",),
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
    ("decl_split_sweep", ["--self-test"]),
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
                        # ours-vs-ours: the damaged copy against the UNDAMAGED
                        # shadow of the same object, so the ONLY difference is
                        # the mutation.  Against retail it would report the
                        # pre-existing `.comment` delta and look like a pass
                        # for entirely the wrong reason.
                        return run([PY, os.path.join(r, "tools",
                                                     "obj_equal.py"),
                                    os.path.join(base_root, "build", "GSAE01",
                                                 "src", *unit.split("/")[1:])
                                    + ".o",
                                    os.path.join(r, "build", "GSAE01", "src",
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

    # THE CLI IS A SEPARATE SURFACE.  direct_build was import-only, with no
    # `__main__` at all: a shell caller got a silent no-op and exit 0, then
    # measured whatever object the PREVIOUS probe had left behind.  So the CLI
    # must (a) actually rebuild a deleted object, and (b) FAIL, not succeed,
    # when it cannot -- the second half is the control, because a no-op passes
    # (a) trivially once the object is already there.
    dbp = os.path.join(REPO, "tools", "direct_build.py")
    os.unlink(src_o)
    rc_ok, _ = run([PY, dbp, unit_object])
    built = os.path.exists(src_o) and md5(src_o) == before
    ok = rc_ok == 0 and built
    bad += not ok
    print("  [%s] ...and its CLI rebuilds a DELETED object (rc=%s, rebuilt=%s)"
          % ("held" if ok else "SILENT NO-OP", rc_ok, built))
    rc_bad, _ = run([PY, dbp, "build/GSAE01/obj/main/__no_such_unit__.o"])
    rc_none, _ = run([PY, dbp])
    ok = rc_bad != 0 and rc_none != 0
    bad += not ok
    print("  [%s] ...and it reports FAILURE for a target it cannot build "
          "(rc=%s) and for no argument at all (rc=%s)"
          % ("held" if ok else "EXITS 0 ANYWAY", rc_bad, rc_none))
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

    # THE PATH THE SWEEPERS ACTUALLY TAKE.  Neither surface above is it: every
    # sweeper calls `brute_match.rebuild`, which does the obj/->src/ rewrite and
    # an env override of its own before it ever reaches direct_build.  The BUILD
    # family passed for months while the CLI was a no-op precisely because it
    # controlled a surface no caller used, so this half mutates the SOURCE and
    # requires the object to move -- the only question that distinguishes a
    # build tool from a program that does nothing.
    import brute_match as _bm                                 # noqa: E402
    _cfg = json.load(open(os.path.join(REPO, "build", "GSAE01", "config.json")))
    _name = next((u["name"] for u in _cfg["units"]
                  if u["object"].replace("\\", "/") == unit_object), None)
    src_c = os.path.join(REPO, "src", _name.replace("\\", "/")) if _name else None
    if src_c and os.path.exists(src_c):
        with open(src_c, "rb") as fh:
            pristine = fh.read()
        try:
            with open(src_c, "wb") as fh:
                fh.write(pristine + b"\nint _vacuityProbeFn(int x) { return x * 37 + 11; }\n")
            moved = _bm.rebuild(unit_object, "GSAE01") and md5(src_o) != before
            with open(src_c, "wb") as fh:
                fh.write(pristine + b"\nthis is not C;\n")
            refused = _bm.rebuild(unit_object, "GSAE01") is False
        finally:
            with open(src_c, "wb") as fh:
                fh.write(pristine)
            _bm.rebuild(unit_object, "GSAE01")
        restored = os.path.exists(src_o) and md5(src_o) == before
        for ok_, msg in ((moved, "brute_match.rebuild (the CALLER path) sees a source change"),
                         (refused, "...and returns False when the source cannot compile"),
                         (restored, "...and the restored source rebuilds the original bytes")):
            bad += not ok_
            print("  [%s] %s" % ("held" if ok_ else "VACUOUS", msg))

    # objdiff OMITS fuzzy_match_percent at 0.0.  `unitfuzzy`'s CLI reads it with
    # a default; `fuzzy_measure`, which is the API every sweep ranks on, did not
    # -- so a variant that zeroed the function raised KeyError and killed the
    # sweep, and an aborted sweep reads exactly like an exhausted one.  Same
    # shape as the direct_build defect with the surfaces swapped.
    _real_measure = unitfuzzy.measure
    unitfuzzy.measure = lambda unit, version: {"functions": [{"name": "probeSym"}]}
    try:
        try:
            got = _bm.fuzzy_measure({"object": unit_object, "name": "x"},
                                    "probeSym", "GSAE01", retries=1)
            handled = isinstance(got, float)
        except Exception:
            handled = False
        # ABLATION: the pre-fix unguarded read must still blow up, or the
        # tolerance above is somebody else's doing.
        try:
            u_ = unitfuzzy.measure(None, None)
            float(u_["functions"][0]["fuzzy_match_percent"])
            ablated_raises = False
        except KeyError:
            ablated_raises = True
    finally:
        unitfuzzy.measure = _real_measure
    for ok_, msg in ((handled, "fuzzy_measure survives an OMITTED fuzzy key (a 0.0 row) "
                               "instead of killing the sweep"),
                     (ablated_raises, "...and ablating the guard raises KeyError again")):
        bad += not ok_
        print("  [%s] %s" % ("held" if ok_ else "VACUOUS", msg))

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

    # THE SAME TRAP, EVERYWHERE ELSE IT WAS PASTED.  `fuzzy_measure` was fixed
    # where the defect was noticed; `stmt_sweep`, `permsweep` and
    # `probe_spelling` each carried their own copy of the identical unguarded
    # read and none of them was touched.  A fix applied at one call site is not
    # a fix, so the rule is checked over every tool that RANKS a probe: either
    # the file never subscripts the key, or it tests for the key first (the
    # `score_delta_gate` / `flag_probe` shape).
    import ast

    def unguarded(text):
        try:
            tree = ast.parse(text)
        except SyntaxError:
            return False
        subs = [n for n in ast.walk(tree)
                if isinstance(n, ast.Subscript)
                and isinstance(getattr(n, "slice", None), ast.Constant)
                and n.slice.value == "fuzzy_match_percent"]
        if not subs:
            return False
        return '"fuzzy_match_percent" in ' not in text and \
               '"fuzzy_match_percent" not in ' not in text

    RANKERS = ("brute_match.py", "decl_split_sweep.py", "stmt_sweep.py",
               "expr_sweep.py", "perm_solve.py", "deep_decl.py",
               "slot_oracle.py", "permsweep.py", "probe_spelling.py",
               "flag_probe.py", "batch_brute.py", "operand_sweep.py",
               "score_delta_gate.py", "unitfuzzy.py")
    offenders = []
    checked_files = 0
    for tool in RANKERS:
        path = os.path.join(REPO, "tools", tool)
        if not os.path.exists(path):
            continue
        checked_files += 1
        if unguarded(open(path).read()):
            offenders.append(tool)
    ok = not offenders and checked_files >= 10
    bad += not ok
    print("  [%s] no ranking tool reads fuzzy_match_percent unguarded "
          "(%d files checked)%s"
          % ("held" if ok else "UNGUARDED", checked_files,
             "" if not offenders else "  *** " + ", ".join(offenders)))

    # ABLATION: the scan must SEE one.  A checker that finds nothing because it
    # is looking in the wrong place is indistinguishable from a clean tree.
    saw = unguarded("def f(r):\n    return float(r['fuzzy_match_percent'])\n")
    quiet = unguarded("def f(r):\n    return float(r.get('fuzzy_match_percent') or 0)\n")
    guarded_ok = unguarded('def f(r):\n'
                           '    if "fuzzy_match_percent" in r:\n'
                           '        return float(r["fuzzy_match_percent"])\n')
    ok = saw and not quiet and not guarded_ok
    bad += not ok
    print("  [%s] ...and the scan catches an injected unguarded read, passes "
          "`.get`, and passes an `in`-guarded subscript (%s/%s/%s)"
          % ("held" if ok else "BLIND", saw, quiet, guarded_ok))
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


_PERTURB = """
import sys;sys.path.insert(0,'tools')
from pathlib import Path
import brute_match as B
from function_objdump import load_units, resolve_unit
u=resolve_unit(load_units(Path('build/GSAE01/config.json')), %r)
f=Path('src')/u['name']
src=f.read_bytes().decode('latin-1')
o,e=B.find_function_body(src, %r)
bl=B.collect_decl_blocks(src,o,e)[0]
order=list(range(len(bl['items']))); order[%d],order[%d]=order[%d],order[%d]
it=bl['items']
first=it[order[0]].lstrip()
rest=''.join('\\n'+bl['indent']+it[k].lstrip() for k in order[1:])
f.write_bytes((src[:bl['start']]+first+rest+src[bl['end']:]).encode('latin-1'))
"""


def family_oracle(verbose):
    """POSITIVE CONTROL for tools/slot_oracle.py.

    Swept over the sub-100 rows the oracle answers "pi not realisable within
    this block" essentially every time.  A uniform verdict is exactly the shape
    a broken instrument produces, so the census means nothing until the oracle
    has been seen saying something else AND being right.

    Perturb a function that is already at 100.0 by swapping two of its
    declarations: our object now differs from retail by precisely the register
    transposition that swap caused, which is a pi that IS realisable and whose
    solution is the swap back.  (An unperturbed 100.0 function is useless --
    pi is empty and the oracle short-circuits before it probes anything, which
    is why the perturbation has to come first.)

    NOTE: writes into src/ and rebuilds.  Restores from HEAD after every probe.
    Do not run against a worktree with a sweep in flight.
    """
    print("\n=== POSITIVE CONTROL: tools/slot_oracle.py ===")
    rep = json.load(open(os.path.join(REPO, "build/GSAE01/report.json")))
    cfg = json.load(open(os.path.join(REPO, "build/GSAE01/config.json")))
    names = {u["name"] for u in cfg["units"]}
    cands = []
    for u in rep["units"]:
        for f in u.get("functions") or []:
            if f.get("fuzzy_match_percent") != 100.0:
                continue
            if not (1200 <= int(f.get("size", 0)) <= 6000):
                continue
            base = "/".join(u["name"].split("/")[1:]) + ".c"
            if base in names and "dolphin" not in base and "musyx" not in base:
                cands.append((base, f["name"], int(f["size"])))
    cands.sort(key=lambda r: -r[2])

    def restore(rel):
        g = subprocess.run(["git", "show", "HEAD:src/" + rel], cwd=REPO,
                           capture_output=True)
        if g.returncode == 0:
            open(os.path.join(REPO, "src", rel), "wb").write(g.stdout)
        b = os.path.join(REPO, "src", rel) + ".brutebak"
        if os.path.exists(b):
            os.unlink(b)

    tried = 0
    for cfgname, fn, size in cands:
        r = run([PY, os.path.join(REPO, "tools", "brute_match.py"),
                 cfgname, fn, "--list-blocks"])
        m = re.search(r"block 0: depth=\d+ line \d+-\d+\s+(\d+) items", r[1])
        if not m or int(m.group(1)) < 6:
            continue
        n = int(m.group(1))
        for (i, j) in ((0, 1), (0, 2), (1, 2), (2, 3)):
            tried += 1
            if tried > 24:
                print("  *** no subject produced a positive control in 24 "
                      "perturbations ***")
                return 1
            w = run([PY, "-c", _PERTURB % (cfgname, fn, i, j, j, i)])
            if w[0]:
                restore(cfgname)
                continue
            rc, txt = run([PY, os.path.join(REPO, "tools", "slot_oracle.py"),
                           cfgname, fn])
            restore(cfgname)
            b = re.search(r"baseline fuzzy=([\d.]+)", txt)
            if not b or float(b.group(1)) >= 100.0:
                continue                    # the swap was inert; try another
            pi = bool(re.search(r"# pi \(saved band\): \{.+\}", txt))
            unreal = "not realisable" in txt
            sol = re.search(r"SOLVED order \[[^\]]*\] -> fuzzy=([\d.]+)", txt)
            back = bool(sol) and abs(float(sol.group(1)) - 100.0) < 1e-9
            print("  %s %s: perturbed decl %d<->%d, fuzzy 100.0 -> %s;  "
                  "pi non-empty=%s  unrealisable=%s  solved-back-to-100=%s"
                  % (cfgname, fn, i, j, b.group(1), pi, unreal, back))
            if pi and not unreal and back:
                print("  [held] the oracle CAN report a realisable pi, and the "
                      "ordering it computes is the right one")
                return 0
    print("  *** NO SUBJECT PRODUCED A POSITIVE CONTROL ***")
    return 1


def family_parser(verbose):
    """The DECLARATION PARSER every ordering sweep permutes through.

    `brute_match.looks_like_decl` decides what a "declaration block" contains,
    and `brute_match`, `slot_oracle`, `stmt_sweep`, `perm_solve`, `deep_decl`,
    `expr_sweep` and `decl_split_sweep` all permute exactly what it hands them.
    If it accepts a STATEMENT, the sweepers reorder side-effecting stores
    against each other -- and NO gate in this project can see that, because
    objdiff fuzzy, obj_equal, score_delta_gate and the forced link all compare
    retail's BYTES and never its MEANING.  A reordered pair of stores that
    happened to score better would have landed as "a declaration ordering".

    So the parser needs a control of its own: subjects that MUST be refused,
    and an ablation that removes the guard and shows the refusal was the guard
    doing work rather than something else declining them anyway.
    """
    print("\n=== PARSER (what the ordering sweeps are allowed to permute) ===")
    sys.path.insert(0, os.path.join(REPO, "tools"))
    import brute_match as B                                   # noqa: E402
    bad = 0

    must_refuse = [
        "tri->edgeOutBits = 0;",
        "cfg.startPosX += spawnParams->posX;",
        "slot->scaleTarget = slot->scaleCurrent;",
        "obj->anim.localPosX = knockbackDistance * -obj->anim.velocityX;",
        "state->dirX /= length;",
    ]
    must_accept = [
        "int x;",
        "GameObject* obj = p;",
        "f32 *a, *b;",
        "u8 buf[4];",
        "ShaderRomListSlot* romListSlot = (ShaderRomListSlot*)(base + 4) + i;",
    ]
    ref = [s for s in must_refuse if B.looks_like_decl(s)]
    acc = [s for s in must_accept if not B.looks_like_decl(s)]
    bad += bool(ref) + bool(acc)
    print("  [%s] a member-access STORE is not a declaration  (%d/%d refused)"
          % ("held" if not ref else "ACCEPTED", len(must_refuse) - len(ref),
             len(must_refuse)))
    for s in ref:
        print("        *** accepted: %s" % s)
    print("  [%s] a real declaration still parses  (%d/%d accepted)"
          % ("held" if not acc else "REFUSED", len(must_accept) - len(acc),
             len(must_accept)))
    for s in acc:
        print("        *** refused: %s" % s)

    # ABLATION: remove the member-access guard and require the subjects to be
    # accepted again.  If they are still refused, something else is declining
    # them and the guard is decorative.
    src = open(os.path.join(REPO, "tools", "brute_match.py")).read()
    guard = 'if "->" in head or "." in head:\n        return False'
    if guard not in src:
        print("  [MISSING] the member-access guard is not in brute_match.py")
        return bad + 1
    ns = {"__file__": os.path.join(REPO, "tools", "brute_match.py"),
          "__name__": "brute_match_ablated"}
    exec(compile(src.replace(guard, "pass"), "<ablated>", "exec"), ns)
    ablated = ns["looks_like_decl"]
    revived = [s for s in must_refuse if ablated(s)]
    ok = len(revived) == len(must_refuse)
    bad += not ok
    print("  [%s] ablating the guard makes the parser accept all %d stores "
          "again (%d)" % ("held" if ok else "VACUOUS", len(must_refuse),
                          len(revived)))

    # THE SAME HAZARD, ONE STEP PAST THE PARSER.  Everything above asks "is
    # this a declaration".  What the sweeps actually depend on is "is this
    # permutation semantics-preserving", and for a declaration whose
    # INITIALISER calls something the two come apart: it is a declaration by
    # every test the parser applies, and swapping two of them changes the
    # computation exactly the way a swapped pair of stores would.  So the
    # detector that guards the apply gate gets its own subjects.
    must_flag = [
        "s32 rnd1 = randomGetRange(0, 0x1e) * 2;",
        "s32 rnd2 = randomGetRange(0, 0x1e) * 2;",
    ]
    must_not_flag = [
        "int i;",
        "u64 x = (u32)(p);",
        "int n = a->count;",
        "f32 v = base[i] * 0.5f;",
    ]
    got = [B.side_effect_reorders(must_flag, [1, 0]),
           B.side_effect_reorders(must_flag, [0, 1])]
    ok = got[0] == [(0, 1)] and got[1] == []
    bad += not ok
    print("  [%s] swapping two PRNG-draw initialisers is flagged, and the "
          "identity is not  (%s)" % ("held" if ok else "BLIND", got))
    quiet = [t for t in must_not_flag if B.initialiser_calls(t)]
    ok = not quiet
    bad += not ok
    print("  [%s] a plain declaration, a cast and a field read carry no call "
          "(%d/%d quiet)" % ("held" if not quiet else "FALSE POSITIVE",
                             len(must_not_flag) - len(quiet), len(must_not_flag)))
    for t in quiet:
        print("        *** flagged: %s" % t)

    # ABLATION: a detector that never finds a call reports nothing at all and
    # looks exactly like a clean tree, so blind the call scanner and require
    # the flag to disappear.
    saved_re = B.CALL_TOKEN_RE
    try:
        B.CALL_TOKEN_RE = re.compile(r"(?!x)x")
        blinded = B.side_effect_reorders(must_flag, [1, 0])
    finally:
        B.CALL_TOKEN_RE = saved_re
    ok = blinded == []
    bad += not ok
    print("  [%s] blinding the call scanner makes the flag vanish, so the "
          "flag comes from the CALL (%s)"
          % ("held" if ok else "VACUOUS", blinded))

    # And the gate must be wired into both sweepers, not merely defined.
    wired = 0
    for tool in ("brute_match.py", "decl_split_sweep.py"):
        txt = open(os.path.join(REPO, "tools", tool)).read()
        if "report_side_effect_reorders" in txt and \
           "allow_side_effect_reorder" in txt:
            wired += 1
    ok = wired == 2
    bad += not ok
    print("  [%s] the apply gate is wired into both sweepers (%d/2)"
          % ("held" if ok else "DEFINED BUT UNUSED", wired))

    # And the ablation must be VISIBLE where it matters: a block whose stores
    # are order-dependent.  `slot->scaleCurrent = v; slot->scaleTarget =
    # slot->scaleCurrent;` reads a value the previous store wrote, so swapping
    # the two changes the computation -- which is the whole point.
    dependent = ("    slot->scaleCurrent = scaleVal;\n"
                 "    slot->scaleTarget = slot->scaleCurrent;\n")
    body = "void f(void) {\n" + dependent + "    g();\n}\n"
    o, e = B.find_function_body(body, "f")
    now = B.collect_decl_blocks(body, o, e)
    then = ns["collect_decl_blocks"](body, o, e)
    ok = (not now) and len(then) == 1 and len(then[0]["items"]) == 2
    bad += not ok
    print("  [%s] an ORDER-DEPENDENT pair of stores is no longer a permutable "
          "block (was %d item(s), now %d)"
          % ("held" if ok else "STILL PERMUTABLE",
             len(then[0]["items"]) if then else 0,
             len(now[0]["items"]) if now else 0))
    return bad


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--family",
                    choices=("scope", "object", "delegate", "build",
                             "report", "oracle", "parser"),
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
        print("scope:    " + " ".join(SCOPE_SCREENS))
        print("object:   " + " ".join(sorted(OBJECT_SCREENS)))
        print("delegate: " + " ".join(t for t, _ in DELEGATED))
        print("build:    direct_build unitfuzzy   (opt-in: writes objects)")
        print("report:   build/GSAE01/report.json")
        print("oracle:   slot_oracle              (opt-in: writes source)")
        print("parser:   brute_match.looks_like_decl -- what every ordering "
              "sweep is allowed to permute")
        return 0
    fams = a.family or ["scope", "object", "delegate", "report",
                        "parser"]
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
    if "oracle" in fams:
        bad += family_oracle(a.verbose)
    if "parser" in fams:
        bad += family_parser(a.verbose)
    print("\nvacuity audit: %s" % ("ALL INSTRUMENTS SCREAMED" if not bad
                                   else "%d INSTRUMENT(S) STAYED SILENT" % bad))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
