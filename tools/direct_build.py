#!/usr/bin/env python3
"""Lock-free single-object rebuild for parallel sweeps.

`tools/locked_ninja.sh` takes a GLOBAL directory mutex, so every probe of every
concurrent sweep serialises on one ninja invocation.  Measured on a 10-core box:
ten `brute_match.py` workers hold a load average of 3.9 -- roughly 60% of the
fleet is asleep waiting for the lock, and the lock is only protecting ninja's
own `.ninja_log`/`.ninja_deps`, not the compiler.

A probe does not need ninja.  Each worker compiles a DIFFERENT unit, and the
compile command for a unit is fixed, so `ninja -t commands <target>` (once, under
the lock) yields a command line that can then be run directly, in parallel, with
no shared state at all.  Only the LAST command of that listing builds the target;
the earlier lines are the `download_tool` edges for `wibo`/`sjiswrap`, which must
never be re-run (the tool host answers HTTP 500).

The cache is per-process and keyed on the object path, so a sweep pays the ninja
startup once per unit instead of once per probe.

    from direct_build import direct_build
    ok = direct_build("build/GSAE01/src/main/objhits.o")

Falls back to `tools/locked_ninja.sh` when the command cannot be recovered, so a
caller can switch to it unconditionally.
"""
from __future__ import annotations

import subprocess
import threading
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent

_cache: dict[str, str] = {}
_lock = threading.Lock()


def compile_command(target: str) -> str | None:
    with _lock:
        if target in _cache:
            return _cache[target]
    r = subprocess.run(["bash", "--noprofile", "--norc",
                        "tools/locked_ninja.sh", "-t", "commands", target],
                       cwd=REPO, capture_output=True, text=True)
    if r.returncode != 0:
        return None
    lines = [l for l in r.stdout.splitlines() if l.strip()]
    if not lines:
        return None
    cmd = lines[-1]
    if "download_tool.py" in cmd:
        return None
    with _lock:
        _cache[target] = cmd
    return cmd


def direct_build(unit_object: str, version: str = "GSAE01") -> bool:
    rel = unit_object.replace(f"build/{version}/obj/", f"build/{version}/src/")
    out = REPO / rel
    try:
        out.unlink()
    except FileNotFoundError:
        pass
    cmd = compile_command(rel)
    if cmd is None:
        r = subprocess.run(["bash", "--noprofile", "--norc",
                            "tools/locked_ninja.sh", rel],
                           cwd=REPO, capture_output=True, text=True)
        return r.returncode == 0 and out.is_file()
    r = subprocess.run(["bash", "--noprofile", "--norc", "-c", cmd],
                       cwd=REPO, capture_output=True, text=True)
    return r.returncode == 0 and out.is_file()
