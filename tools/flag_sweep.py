"""Sweep TU-level compiler profiles for whole units and rank them by residual.

Per-function optimization pragmas are banned (see CLAUDE.md); the only sanctioned
way to give a translation unit a different optimization profile is a TU-level
cflag entry in `configure.py`.  Finding the profile that actually helps means
trying them.  This tool does that mechanically: it takes a unit's REAL ninja
compile command, mutates the flags, compiles to a scratch directory, and scores
the result with `tools/resid_rank.py`'s link-equivalence screen.

    python3 tools/flag_sweep.py main/objprint.c              # 26 single profiles
    python3 tools/flag_sweep.py --pairs main/objprint.c      # 21 -opt subflag pairs
    python3 tools/flag_sweep.py --versions main/objprint.c   # compiler versions only
    python3 tools/flag_sweep.py --all --jobs 8               # every NonMatching unit
    python3 tools/flag_sweep.py --list                       # show profile names

Nothing is written to the tree: `configure.py` is never touched and the unit's
real `.o` is never overwritten.  A hit here is a LEAD, not a landing.

!! TWO-METRIC RULE -- READ BEFORE LANDING ANYTHING THIS TOOL FINDS !!

    The score printed here is RESIDUAL: raw bytes plus relocation differences
    that mwld would still have to reconcile (`score = bytes + 4*relocs`).  Only
    residual ZERO flips a unit to `complete`.  Residual and report.json's
    `fuzzy_match_percent` move INDEPENDENTLY, and routinely in OPPOSITE
    directions -- one observed sweep cut a unit's residual 4259 -> 1803 while
    fuzzy collapsed 99.478 -> 74.51.

    So: for every profile you intend to land, edit `configure.py`, rebuild, and
    regenerate a FULL `report.json`.  Land it only if residual improved AND
    fuzzy did not regress.  Never land on this tool's number alone.
"""
from __future__ import annotations

import argparse
import itertools
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(os.environ.get("SFA_ROOT", Path(__file__).resolve().parents[1]))
sys.path.insert(0, str(ROOT / "tools"))

import resid_rank as RR  # noqa: E402

# -opt subflags worth toggling; the pairwise mode takes combinations of these.
OPT_SUBFLAGS = ["nocse", "nodeadstore", "nolifetimes", "noloopinvariants",
                "nostrength", "nopropagation", "nodead"]
VERSIONS = ["1.2.5n", "1.3", "1.3.2", "2.0", "2.5", "2.6", "2.7"]


def add_opt(*extras):
    def m(toks):
        toks = list(toks)
        if "-opt" in toks:
            i = toks.index("-opt")
            cur = toks[i + 1].split(",")
            toks[i + 1] = ",".join(cur + [e for e in extras if e not in cur])
        else:
            toks = toks[:1] + ["-opt", ",".join(extras)] + toks[1:]
        return toks
    return m


def del_opt(sub):
    def m(toks):
        toks = list(toks)
        if "-opt" in toks:
            i = toks.index("-opt")
            parts = [p for p in toks[i + 1].split(",") if p != sub]
            if parts:
                toks[i + 1] = ",".join(parts)
            else:
                del toks[i:i + 2]
        return toks
    return m


def set_olevel(level):
    def m(toks):
        toks = [t for t in toks if not re.match(r"^-O[0-4]", t)]
        return toks[:1] + [level] + toks[1:]
    return m


def set_inline(value):
    def m(toks):
        toks = list(toks)
        while "-inline" in toks:
            i = toks.index("-inline")
            del toks[i:i + 2]
        if value:
            toks = toks[:1] + ["-inline", value] + toks[1:]
        return toks
    return m


def set_version(ver):
    pat = re.compile(r"build/compilers/GC/[^/]+/")
    def m(toks):
        return [pat.sub("build/compilers/GC/%s/" % ver, t) for t in toks]
    return m


def _single_profiles():
    p = [("+" + s, add_opt(s)) for s in OPT_SUBFLAGS]
    p += [("-nopeephole", del_opt("nopeephole")), ("+nopeephole", add_opt("nopeephole")),
          ("-noschedule", del_opt("noschedule")), ("+noschedule", add_opt("noschedule"))]
    p += [(lvl.lstrip("-").replace(",", ""), set_olevel(lvl))
          for lvl in ("-O4,p", "-O3,p", "-O2,p", "-O4")]
    p += [("inl_noauto", set_inline("noauto")), ("inl_auto", set_inline("auto")),
          ("inl_nodef", set_inline("noauto,deferred")), ("inl_off", set_inline("off"))]
    return p


def _version_profiles():
    return [("v" + v, set_version(v)) for v in VERSIONS]


def _pair_profiles():
    return [("+".join(c), add_opt(*c)) for c in itertools.combinations(OPT_SUBFLAGS, 2)]


def profiles_for(mode):
    if mode == "pairs":
        return _pair_profiles()
    if mode == "versions":
        return _version_profiles()
    return _single_profiles() + _version_profiles()


def base_command(obj):
    """The unit's real mwcc invocation, straight out of ninja."""
    out = subprocess.run(["ninja", "-t", "commands", obj], cwd=ROOT,
                         capture_output=True, text=True).stdout
    for line in out.splitlines():
        if "mwcceppc.exe" in line:
            return line.split("&&")[0].strip()
    return None


def compile_and_score(cmd, unit, mutate, target, workdir):
    toks = shlex.split(cmd)
    for i, t in enumerate(toks):
        if t == "-o":
            toks[i + 1] = str(workdir) + "/"
    toks = [t for t in toks if t != "-MMD"]
    toks = mutate(toks)
    for f in os.listdir(workdir):
        if f.endswith(".o"):
            os.remove(os.path.join(workdir, f))
    subprocess.run(toks, cwd=ROOT, capture_output=True, text=True)
    obj = os.path.join(workdir, os.path.basename(unit)[:-2] + ".o")
    if not os.path.exists(obj):
        return None
    try:
        nbytes, nrelocs, _ = RR.screen(obj, target)
    except Exception:
        return None
    return nbytes, nrelocs


def sweep_unit(unit, obj, target, profs, top):
    cmd = base_command(obj)
    if not cmd:
        return f"{unit}: NO COMPILE COMMAND"
    workdir = tempfile.mkdtemp(prefix="flagsweep.")
    try:
        base = compile_and_score(cmd, unit, lambda t: t, target, workdir)
        if base is None:
            return f"{unit}: BASELINE BUILD FAILED"
        base_score = base[0] + 4 * base[1]
        hits = []
        for name, mutate in profs:
            got = compile_and_score(cmd, unit, mutate, target, workdir)
            if got is None:
                continue
            score = got[0] + 4 * got[1]
            if score < base_score:
                hits.append((score, name, got))
        hits.sort()
    finally:
        shutil.rmtree(workdir, ignore_errors=True)
    body = "  ".join(f"{n}={s}({g[0]}b/{g[1]}r)" for s, n, g in hits[:top]) or "-"
    return f"{unit}: base={base_score}({base[0]}b/{base[1]}r)  {body}"


def resolve(selectors, only_incomplete):
    for name, ours, target, (done, _fz, _s1) in RR.units(tuple(selectors)):
        if only_incomplete and done:
            continue
        obj = str(Path(ours).relative_to(ROOT))
        yield name, obj, target


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("units", nargs="*", help="unit name or substring, e.g. main/objprint.c")
    ap.add_argument("--all", action="store_true",
                    help="sweep every NonMatching unit instead of a named list")
    ap.add_argument("--pairs", action="store_true",
                    help="sweep the 21 pairwise -opt subflag combinations")
    ap.add_argument("--versions", action="store_true",
                    help="sweep compiler versions only")
    ap.add_argument("--top", type=int, default=5, help="hits to print per unit")
    ap.add_argument("--jobs", type=int, default=1, help="units to sweep in parallel")
    ap.add_argument("--list", action="store_true", help="print profile names and exit")
    args = ap.parse_args()

    mode = "pairs" if args.pairs else "versions" if args.versions else "single"
    profs = profiles_for(mode)
    if args.list:
        print(" ".join(n for n, _ in profs))
        return
    if not args.units and not args.all:
        ap.error("name at least one unit, or pass --all")

    targets = list(resolve(args.units, args.all))
    if not targets:
        ap.error("no unit matched")

    def work(t):
        return sweep_unit(t[0], t[1], t[2], profs, args.top)

    if args.jobs > 1:
        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            for line in pool.map(work, targets):
                print(line)
                sys.stdout.flush()
    else:
        for t in targets:
            print(work(t))
            sys.stdout.flush()


if __name__ == "__main__":
    main()
