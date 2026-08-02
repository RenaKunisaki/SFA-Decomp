#!/usr/bin/env python3
"""Rank non-matching functions by how few instruction words actually differ.

bandscreen.py answers "is the saved band steerable"; this answers "how close is
this function to byte-exact, and is the residual a shape change or a recolour".
For each non-matching function it disassembles the target and our object,
compares instruction words positionally, and reports:

    ndiff   words that differ at all
    struc   of those, how many changed MNEMONIC, plus any length difference

A low ndiff with struc == 0 is a register/operand permutation.  A low ndiff with
struc > 0 is a real shape difference, which is where the source levers apply.
#nm is how many functions in the unit still differ, so #nm == 1 means matching
this one function flips the whole unit.
"""
import argparse
import difflib
import json
import re
import subprocess
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OD = os.path.join(ROOT, "build/binutils/powerpc-eabi-objdump")


def words(obj, sym):
    out = subprocess.run([OD, "-M", "gekko", "-drz", "--disassemble=" + sym, obj],
                         capture_output=True, text=True).stdout
    got = []
    for line in out.splitlines():
        m = re.match(r'\s*[0-9a-f]+:\s+((?:[0-9a-f]{2} ){4})\s*(\S+)', line)
        if m:
            got.append((m.group(1), m.group(2)))
    return got


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--min-size", type=int, default=500,
                    help="ignore functions smaller than this many bytes")
    ap.add_argument("--max-ndiff", type=int, default=0,
                    help="only show functions with at most this many differing words (0 = no limit)")
    ap.add_argument("--sole", action="store_true",
                    help="only functions that are the last straggler in their unit")
    args = ap.parse_args()

    report = json.load(open(os.path.join(ROOT, "build/GSAE01/report.json")))
    cfg = json.load(open(os.path.join(ROOT, "objdiff.json")))
    by_name = {u["name"].replace("\\", "/"): u for u in cfg["units"]}

    rows = []
    for unit in report["units"]:
        if unit.get("metadata", {}).get("auto_generated"):
            continue
        cu = by_name.get(unit["name"].replace("\\", "/"))
        if not cu or not cu.get("base_path"):
            continue
        fns = [f for f in (unit.get("functions") or []) if f]
        remaining = sum(1 for f in fns if f.get("fuzzy_match_percent", 100) < 100)
        if args.sole and remaining != 1:
            continue
        for f in fns:
            fuzzy = f.get("fuzzy_match_percent", 100.0)
            size = int(f.get("size", 0))
            if fuzzy >= 100.0 or size < args.min_size:
                continue
            a = words(os.path.join(ROOT, cu["target_path"]), f["name"])
            b = words(os.path.join(ROOT, cu["base_path"]), f["name"])
            if not a or not b:
                continue
            sm = difflib.SequenceMatcher(None, [w for w, _ in a], [w for w, _ in b], autojunk=False)
            diff = []
            for tag, i1, i2, j1, j2 in sm.get_opcodes():
                if tag != "equal":
                    diff.extend(range(i1, i2 if i2 > i1 else i1 + 1))
            n = min(len(a), len(b))
            if args.max_ndiff and len(diff) > args.max_ndiff:
                continue
            struc = sum(1 for i in diff if i < n and a[i][1] != b[i][1]) + abs(len(a) - len(b))
            rows.append((size, fuzzy, len(diff), struc, unit["name"], f["name"], remaining))

    rows.sort(reverse=True)
    print("%6s %9s %5s %5s %4s  %s" % ("size", "fuzzy", "ndiff", "struc", "#nm", "fn"))
    for size, fuzzy, ndiff, struc, unit, fn, remaining in rows:
        print("%6d %9.5f %5d %5d %4d  %-42s %s" % (size, fuzzy, ndiff, struc, remaining, fn, unit))


if __name__ == "__main__":
    main()
