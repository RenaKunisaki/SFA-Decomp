#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import shutil
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "GSAE01",  # 0
    "GSAJ01",  # 1
    "GSAP01",  # 2
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--ninja",
    metavar="BINARY",
    type=Path,
    help="path to ninja binary (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--matching",
    dest="non_matching",
    action="store_false",
    help="build matching objects and use the hash-checked default target",
)
parser.add_argument(
    "--warn",
    dest="warn",
    type=str,
    choices=["all", "off", "error"],
    help="how to handle warnings",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
parser.add_argument(
    "--zlb-toolchain",
    dest="zlb_toolchain",
    type=str,
    choices=["prodg", "mwcc"],
    default="prodg",
    help="compiler for src/main/zlb.c; mwcc is a diagnostic comparison path "
    "only (retail is GCC-family)",
)
parser.add_argument(
    "--prodg-version",
    dest="prodg_version",
    type=str,
    default="3.5",
    help="ProDG release under build/compilers/ProDG when --zlb-toolchain=prodg",
)
parser.set_defaults(non_matching=True)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.ninja_path = args.ninja
if config.ninja_path is None:
    ninja_path = shutil.which("ninja")
    if ninja_path is not None:
        config.ninja_path = Path(ninja_path)
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-1"
config.compilers_tag = "20251118"
config.dtk_tag = "v1.8.0"
config.objdiff_tag = "v3.5.1"
config.sjiswrap_tag = "v1.2.2"
config.wibo_tag = "1.1.0"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym BUILD_VERSION={version_num}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    config.ldflags.append("-mapunused")
    # config.ldflags.append("-listclosure") # For Wii linkers

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []
config.split_deps = [
    Path("config") / config.version / "splits.txt",
    Path("config") / config.version / "symbols.txt",
]

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = None

# Foreign-toolchain rules. zlbDecompress is GCC-family, not MWCC: retail
# carries "mcrxr cr0; addme." doloops, an idiom absent from the whole GC/2.0
# refcorpus. The vintage is OLDER than anything vendored here - all five SN
# ProDG releases (3.5, 3.5b140, 3.7, 3.8.1, 3.9.3) emit byte-identical
# prologues that open stwu-before-mflr on 8-byte-aligned frames, while retail
# opens mflr-before-stwu on an 84-byte (4-aligned) frame. --prodg-version
# selects the release, so an acquired older cc1 can be tested by dropping it
# in build/compilers/ProDG/<ver>.
# NOTE: prologue shape alone does NOT discriminate MWCC from GCC - the matched
# MWCC twin modelApplyBoneTransform opens mflr/stwu/stw/stmw too. Only the
# mcrxr/addme idiom is decisive.
# NOTE: rule prodg hardcodes its flags and never consumes $cflags, so
# per-object cflags on this unit are silently discarded. The cc1 binary itself
# does honour flags (-O2/-Os/-fno-schedule-insns all change output); an earlier
# "cc1 ignores flags" note conflated the two.
prodg_compilers = Path(args.compilers) if args.compilers else Path("build/compilers")
prodg_binutils = Path(args.binutils) if args.binutils else Path("build/binutils")
prodg_as = prodg_binutils / ("powerpc-eabi-as.exe" if is_windows() else "powerpc-eabi-as")
prodg_dir = prodg_compilers / "ProDG" / args.prodg_version
if is_windows():
    prodg_wrapper = ""
    # Native Windows ninja runs commands without an implicit shell, so the
    # "&&" chain must be wrapped in cmd /c (mirrors the mwcc_*extab rules).
    prodg_shell = "cmd /c "
else:
    prodg_wrapper = f"{args.wrapper} " if args.wrapper else "build/tools/wibo "
    prodg_shell = ""
prodg_implicit = [
    str(prodg_compilers) if args.compilers is None else str(prodg_dir / "cc1.exe"),
    str(prodg_binutils) if args.binutils is None else str(prodg_as),
    *([prodg_wrapper.strip()] if prodg_wrapper else []),
]
config.custom_build_rules = [
    {
        "name": "prodg",
        "command": f"{prodg_shell}{prodg_wrapper}{prodg_dir / 'cpp.exe'} -Iinclude -P $in $basefile.i"
        f" && {prodg_wrapper}{prodg_dir / 'cc1.exe'} $basefile.i"
        " -quiet -O1 -fno-common -frerun-loop-opt -frerun-cse-after-loop -o $basefile.s"
        f" && {prodg_as} -mgekko $basefile.s -o $out",
        "description": "PRODG $out",
    },
]

if args.zlb_toolchain == "prodg":
    zlb_object_kwargs = {
        "custom_rule": "prodg",
        "custom_rule_implicit": prodg_implicit,
    }
else:
    zlb_object_kwargs = {}

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    # "-W all",
    "-O4,p",
    "-inline auto",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-fp_contract on",
    "-str reuse",
    "-multibyte",  # For Wii compilers, replace with `-enc SJIS`
    "-i include",
    f"-i build/{config.version}/include",
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION_{config.version}",
]

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Warning flags
if args.warn == "all":
    cflags_base.append("-W all")
elif args.warn == "off":
    cflags_base.append("-W off")
elif args.warn == "error":
    cflags_base.append("-W error")

# Metrowerks library flags
cflags_runtime = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
    "-inline auto",
]

cflags_runtime_125 = [flag for flag in cflags_runtime if flag != "-gccinc"]

# Game/DLL TUs the original build compiled with the scheduler and peephole
# passes off (a per-TU compiler setting, not a per-function one).
cflags_dll_noopt = [
    *cflags_base,
    "-opt", "nopeephole,noschedule",
]

# ...plus auto-inlining off: functions marked `inline` are still inlined, but
# small non-inline helpers are not auto-inlined (matches the original build,
# which emits calls to trivial getters like Music_GetActivePriority).
cflags_dll_noopt_noautoinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule",
    "-inline", "noauto",
]

cflags_dll_noopt_noautoinline_level3 = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,level=3",
    "-inline", "noauto",
]

cflags_dll_noopt_level1 = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,level=1",
]

cflags_dll_noopt_level2 = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,level=2",
]

cflags_dll_noopt_noautoinline_deferred = [
    *cflags_base,
    "-opt", "nopeephole,noschedule",
    "-inline", "noauto,deferred",
]

cflags_dll_nosched = [
    *cflags_base,
    "-opt", "noschedule",
]

cflags_dll_noopt_nostrength = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nostrength",
]

cflags_dll_noopt_nolifetimes_noloopinv_nostrength = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nolifetimes,noloopinvariants,nostrength",
]

cflags_dll_noopt_nocse_nolifetimes_noloopinv_noprop_nostrength = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nocse,nolifetimes,noloopinvariants,nopropagation,nostrength",
]

# ...plus common-subexpression elimination off (opt_common_subs off).
cflags_dll_noopt_nocse = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nocse",
]

# ...plus inlining off (dont_inline on).
cflags_dll_noopt_nocse_noautoinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nocse",
    "-inline", "noauto",
]

cflags_dll_noopt_nodead_noautoinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nodead",
    "-inline", "noauto",
]

cflags_dll_noopt_nodead_noloopinv_noautoinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nodead,noloopinvariants",
    "-inline", "noauto",
]

cflags_dll_noopt_nocse_nodead_nofpcontract_noautoinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nocse,nodead",
    "-inline", "noauto",
    "-fp_contract", "off",
]

cflags_dll_noopt_nocse_noinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nocse",
    "-inline", "off",
]

# ...plus copy/constant propagation off (opt_propagation off).
cflags_dll_noopt_noprop = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nopropagation",
]

# ...plus strength reduction off (keeps byte-array loop indices as a single indexed IV).
cflags_dll_noopt_noprop_noinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nopropagation",
    "-inline", "noauto",
]

cflags_dll_noopt_noprop_noautoinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nopropagation",
    "-inline", "noauto",
]

cflags_dll_noopt_noprop_nostrength = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nopropagation,nostrength",
]

cflags_dll_noopt_noprop_nostrength_noautoinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nopropagation,nostrength",
    "-inline", "noauto",
]

# ...plus loop-invariant code motion off (opt_loop_invariants off).
cflags_dll_noopt_noloopinv = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,noloopinvariants",
]

cflags_dll_noopt_noloopinv_noautoinline = [
    *cflags_dll_noopt_noloopinv,
    "-inline", "noauto",
]

cflags_dll_noopt_noloopinv_noprop_nospecunroll_noautoinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,noloopinvariants,nopropagation",
    "-inline", "noauto",
    '-pragma "ppc_unroll_speculative off"',
]


cflags_dll_noopt_noloopinv_zerodata = [
    *cflags_dll_noopt_noloopinv,
    '-pragma "explicit_zero_data on"',
]

cflags_dll_noopt_noloopinv_noprop_zerodata = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,noloopinvariants,nopropagation",
    '-pragma "explicit_zero_data on"',
]

# ...plus register-lifetime optimization off (opt_lifetimes off).
cflags_dll_noopt_nolifetimes_noautoinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nolifetimes",
    "-inline", "noauto",
]

cflags_dll_noopt_nocse_noprop_nolifetimes_zerodata_noautoinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nocse,nopropagation,nolifetimes",
    '-pragma "explicit_zero_data on"',
    "-inline", "noauto",
]

cflags_dll_noopt_nolifetimes = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nolifetimes",
]

cflags_dll_noopt_noloopinv_nolifetimes = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,noloopinvariants,nolifetimes",
]

# ...plus dead-code elimination off (opt_dead_code off).
cflags_dll_noopt_noloopinv_nolifetimes_nodead = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,noloopinvariants,nolifetimes,nodead",
]

cflags_dll_noopt_noloopinv_nolifetimes_zerodata = [
    *cflags_dll_noopt_noloopinv_nolifetimes,
    '-pragma "explicit_zero_data on"',
]

cflags_dll_noopt_nolifetimes_noinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nolifetimes",
    "-inline", "off",
]

cflags_dll_nopeep = [
    *cflags_base,
    "-opt", "nopeephole",
]

# noopt (peephole+scheduler off) base, plus additional per-TU passes off.
cflags_dll_noopt_nocse_noprop = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nocse,nopropagation",
]

cflags_dll_noopt_noinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule",
    "-inline", "off",
]

cflags_dll_noopt_noloopinv_noinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,noloopinvariants",
    "-inline", "off",
]

cflags_dll_noopt_nocse_noprop_noinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nocse,nopropagation",
    "-inline", "off",
]

cflags_dll_noopt_nocse_noloopinv = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nocse,noloopinvariants",
]

cflags_dll_noopt_nocse_noloopinv_noautoinline = [
    *cflags_dll_noopt_nocse_noloopinv,
    "-inline", "noauto",
]

cflags_dll_noopt_noprop_noinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nopropagation",
    "-inline", "off",
]

cflags_dll_noopt_nostrength_noinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nostrength",
    "-inline", "off",
]

cflags_dll_noopt_nocse_noprop_noloopinv_noinline = [
    *cflags_base,
    "-opt", "nopeephole,noschedule,nocse,nopropagation,noloopinvariants",
    "-inline", "off",
]

cflags_msl = [
    *cflags_base,
    "-char signed",
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
]

msl_math_extra = ["-schedule", "off"]
msl_math_o0_cflags = [flag for flag in cflags_base if flag != "-O4,p"]

# REL flags
cflags_rel = [
    *cflags_base,
    "-sdata 0",
    "-sdata2 0",
]

cflags_trk = [
    *cflags_base,
    "-sdata 0",
    "-sdata2 0",
    "-inline auto,deferred",
    "-rostr",
    "-char signed",
    "-use_lmw_stmw on",
    "-common off",
]

config.linker_version = "GC/1.3.2"


# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": objects,
    }


# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_rel,
        "progress_category": "game",
        "objects": objects,
    }


Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions


config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    {
        "lib": "Runtime.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "progress_category": "sdk",  # str | List[str]
        "objects": [
            Object(
                MatchingFor("GSAE01"),
                "Runtime.PPCEABI.H/__start.c",
                mw_version="GC/1.2.5n",
                cflags=cflags_runtime_125,
            ),
            Object(MatchingFor("GSAE01"), "Runtime.PPCEABI.H/__mem.c", mw_version="GC/1.3"),
            Object(MatchingFor("GSAE01"), "Runtime.PPCEABI.H/mem_TRK.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/__exception.s"),
            Object(MatchingFor("GSAE01"), "Runtime.PPCEABI.H/__va_arg.c"),
            Object(Matching, "Runtime.PPCEABI.H/global_destructor_chain.c"),
            Object(MatchingFor("GSAE01"), "Runtime.PPCEABI.H/runtime.c"),
            Object(MatchingFor("GSAE01"), "Runtime.PPCEABI.H/__init_cpp_exceptions.cpp"),
            Object(MatchingFor("GSAE01"), "Runtime.PPCEABI.H/fragment.c"),
            Object(MatchingFor("GSAE01"), "Runtime.PPCEABI.H/GCN_mem_alloc.c"),
        ],
    },
    DolphinLib(
        "os",
        [
            Object(MatchingFor("GSAE01"), "dolphin/os/OS.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSAlarm.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSAlloc.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSArena.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSAudioSystem.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSCache.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSContext.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSError.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSExec.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSFont.c", extra_cflags=["-use_lmw_stmw", "on"]),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSInterrupt.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSLink.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSMessage.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSMemory.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSMutex.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSReboot.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSReset.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSResetSW.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSRtc.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSStopwatch.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSSync.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSThread.c", extra_cflags=["-use_lmw_stmw", "on"]),
            Object(MatchingFor("GSAE01"), "dolphin/os/OSTime.c"),
            Object(MatchingFor("GSAE01"), "dolphin/os/__ppc_eabi_init.c"),
        ],
    ),
    DolphinLib(
        "base",
        [
            Object(MatchingFor("GSAE01"), "dolphin/base/PPCArch.c"),
        ],
    ),
    DolphinLib(
        "db",
        [
            Object(MatchingFor("GSAE01"), "dolphin/db/db.c"),
        ],
    ),
    DolphinLib(
        "mtx",
        [
            Object(MatchingFor("GSAE01"), "dolphin/mtx/mtx.c", source="dolphin/mtx/mtx.c", extra_cflags=["-DGEKKO"]),
            Object(MatchingFor("GSAE01"), "dolphin/mtx/mtxvec.c", source="dolphin/mtx/mtxvec.c"),
            Object(MatchingFor("GSAE01"), "dolphin/mtx/vec.c"),
            Object(MatchingFor("GSAE01"), "dolphin/mtx/mtx44.c"),
            Object(NonMatching, "dolphin/mtx/mtx44vec.c"),
            Object(MatchingFor("GSAE01"), "dolphin/mtx/psmtx.c"),
        ],
    ),
    DolphinLib(
        "dvd",
        [
            Object(MatchingFor("GSAE01"), "dolphin/dvd/dvdlow.c"),
            Object(MatchingFor("GSAE01"), "dolphin/dvd/DVDLowFirstRead.c"),
            Object(MatchingFor("GSAE01"), "dolphin/dvd/dvdfs.c", extra_cflags=["-use_lmw_stmw", "on"]),
            Object(MatchingFor("GSAE01"), "dolphin/dvd/dvd.c"),
            Object(MatchingFor("GSAE01"), "dolphin/dvd/dvdqueue.c"),
            Object(MatchingFor("GSAE01"), "dolphin/dvd/dvderror.c"),
            Object(MatchingFor("GSAE01"), "dolphin/dvd/fstload.c"),
            Object(MatchingFor("GSAE01"), "dolphin/dvd/dvdFatal.c"),
            Object(NonMatching, "dolphin/dvd/dvdidutils.c"),
        ],
    ),
    DolphinLib(
        "ai",
        [
            Object(MatchingFor("GSAE01"), "dolphin/ai/ai.c"),
        ],
    ),
    DolphinLib(
        "ar",
        [
            Object(MatchingFor("GSAE01"), "dolphin/ar/ar.c"),
            Object(MatchingFor("GSAE01"), "dolphin/ar/arq.c"),
        ],
    ),
    DolphinLib(
        "dsp",
        [
            Object(MatchingFor("GSAE01"), "dolphin/dsp/dsp.c"),
            Object(MatchingFor("GSAE01"), "dolphin/dsp/dsp_task.c"),
            Object(MatchingFor("GSAE01"), "dolphin/dsp/dsp_debug.c"),
        ],
    ),
    DolphinLib(
        "ax",
        [
            Object(MatchingFor("GSAE01"), "dolphin/ax/AX.c"),
            Object(NonMatching, "dolphin/ax/AXAlloc.c"),
            Object(NonMatching, "dolphin/ax/AXAux.c"),
            Object(NonMatching, "dolphin/ax/AXCL.c"),
            Object(NonMatching, "dolphin/ax/AXComp.c"),
            Object(NonMatching, "dolphin/ax/AXOut.c"),
            Object(NonMatching, "dolphin/ax/AXProf.c"),
            Object(NonMatching, "dolphin/ax/AXSPB.c"),
            Object(NonMatching, "dolphin/ax/AXVPB.c"),
        ],
    ),
    DolphinLib(
        "si",
        [
            Object(MatchingFor("GSAE01"), "dolphin/si/SIBios.c", extra_cflags=["-inline", "all", "-char", "signed"]),
            Object(MatchingFor("GSAE01"), "dolphin/si/SISamplingRate.c", extra_cflags=["-inline", "all", "-char", "signed"]),
        ],
    ),
    DolphinLib(
        "pad",
        [
            Object(MatchingFor("GSAE01"), "dolphin/pad/Padclamp.c"),
            Object(MatchingFor("GSAE01"), "dolphin/pad/Pad.c", extra_cflags=["-DVERSION_GCCP01"]),
        ],
    ),
    DolphinLib(
        "exi",
        [
            Object(MatchingFor("GSAE01"), "dolphin/exi/EXIBios.c"),
            Object(MatchingFor("GSAE01"), "dolphin/exi/EXIUart.c"),
        ],
    ),
    DolphinLib(
        "hio",
        [
            Object(NonMatching, "dolphin/hio/hio.c"),
        ],
    ),
    DolphinLib(
        "mcc",
        [
            Object(NonMatching, "dolphin/mcc/mcc.c"),
            Object(NonMatching, "dolphin/mcc/fio.c"),
        ],
    ),
    DolphinLib(
        "mix",
        [
            Object(NonMatching, "dolphin/mix/mix.c"),
        ],
    ),
    DolphinLib(
        "gx",
        [
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXInit.c", extra_cflags=["-opt", "nopeephole"]),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXFifo.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXMisc.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXLight.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXTextureTables.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXTexture.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXBump.c"),
            Object(MatchingFor("GSAE01"), "main/audio/mcmd_data.c", progress_category="game"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXAttr.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXDisplayList.c", extra_cflags=["-sdata", "16"]),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXFrameBuf.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXDrawTorusRadius.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXDraw.c", extra_cflags=["-fp_contract", "off"]),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXDrawTorusAngle.c"),
            Object(MatchingFor("GSAE01"), "main/audio/adsr_data.c", progress_category="game"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXPerf.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXPixel.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXSave.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXStubs.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXTev.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXTransform.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXGeometry.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXVerifRAS.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXVerifXF.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXVerify.c"),
            Object(MatchingFor("GSAE01"), "dolphin/gx/GXVert.c"),
        ],
    ),
    DolphinLib(
        "card",
        [
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDBios.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDUnlock.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDRdwr.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDBlock.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDDir.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDCheck.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDMount.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDFormat.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDOpen.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDCreate.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDRead.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDWrite.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDDelete.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDStat.c"),
            Object(MatchingFor("GSAE01"), "dolphin/card/CARDNet.c"),
        ],
    ),
    DolphinLib(
        "axfx",
        [
            Object(NonMatching, "dolphin/axfx/chorus.c"),
            Object(NonMatching, "dolphin/axfx/delay.c"),
            Object(NonMatching, "dolphin/axfx/reverb_hi.c"),
            Object(NonMatching, "dolphin/axfx/reverb_hi_4ch.c"),
            Object(MatchingFor("GSAE01"), "dolphin/axfx/reverb_std_callback.c", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(NonMatching, "dolphin/axfx/reverb_std.c"),
        ],
    ),
    {
        "lib": "vi",
        "mw_version": "GC/1.2.5n",
        "cflags": [
            *cflags_base,
            "-use_lmw_stmw on",
        ],
        "progress_category": "sdk",
        "objects": [
            Object(MatchingFor("GSAE01"), "dolphin/vi/vi.c"),
            Object(NonMatching, "dolphin/vi/gpioexi.c"),
            Object(NonMatching, "dolphin/vi/i2c.c"),
            Object(NonMatching, "dolphin/vi/initphilips.c"),
        ],
    },
    DolphinLib(
        "thp",
        [
            Object(MatchingFor("GSAE01"), "dolphin/thp/THPDec.c", mw_version="GC/1.2.5"),
            Object(MatchingFor("GSAE01"), "dolphin/thp/THPAudio.c"),
        ],
    ),
    {
        "lib": "OdemuExi2",
        "mw_version": "GC/1.2.5",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": [
            Object(MatchingFor("GSAE01"), "dolphin/OdemuExi2/DebuggerDriver.c"),
        ],
    },
    DolphinLib(
        "odenotstub",
        [
            Object(MatchingFor("GSAE01"), "dolphin/odenotstub/odenotstub.c"),
        ],
    ),
    {
        "lib": "amcstubs",
        "mw_version": "GC/1.3",
        "cflags": cflags_trk,
        "progress_category": "sdk",
        "objects": [
            Object(MatchingFor("GSAE01"), "dolphin/amcstubs/AmcExi2Stubs.c"),
        ],
    },
    {
        "lib": "TRK_MINNOW_DOLPHIN",
        "mw_version": "GC/1.3",
        "cflags": cflags_trk,
        "progress_category": "sdk",
        "objects": [
            Object(
                MatchingFor("GSAE01"),
                "dolphin/TRK_MINNOW_DOLPHIN/mainloop.c",
                mw_version="GC/1.3.2",
            ),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/nubevent.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/nubinit.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/msg.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/msgbuf.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/serpoll.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/usr_put.c"),
            Object(
                MatchingFor("GSAE01"),
                "dolphin/TRK_MINNOW_DOLPHIN/dispatch.c",
                mw_version="GC/1.3.2",
            ),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/msghndlr.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/support.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/mutex_TRK.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/notify.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/flush_cache.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/mem_TRK.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/targimpl.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/targsupp.s"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/dolphin_trk.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/mpc_7xx_603e.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/main_TRK.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/dolphin_trk_glue.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/targcont.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/target_options.c"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/mslsupp.c"),
            Object(
                MatchingFor("GSAE01"),
                "dolphin/TRK_MINNOW_DOLPHIN/MWTrace.c",
                mw_version="GC/1.2.5n",
                extra_cflags=["-sdata", "8", "-sdata2", "8", "-schedule", "off"],
            ),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/MWCriticalSection_gc.c"),
            Object(MatchingFor("GSAE01"), "dlls/objects/450_DIMSnowBall/DIMSnowBall.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/451_DIMGate/DIMGate.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/452_DIMIceWall/DIMIceWall.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/453_DIMBarrier/DIMBarrier.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_801b1d84.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/main.c", progress_category="sdk"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/CircleBuffer.c", progress_category="sdk"),
            Object(MatchingFor("GSAE01"), "dolphin/TRK_MINNOW_DOLPHIN/main_gdev.c", progress_category="sdk"),
        ],
    },
    DolphinLib(
        "MSL_C",
        [
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/abort_exit.c", mw_version="GC/1.3"),
            Object(
                MatchingFor("GSAE01"),
                "dolphin/MSL_C/PPCEABI/bare/H/alloc.c",
                mw_version="GC/1.3",
                cflags=cflags_msl,
                extra_cflags=["-common", "off", "-inline", "auto,deferred"],
            ),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/ansi_files.c", mw_version="GC/1.3"),
            Object(
                MatchingFor("GSAE01"),
                "dolphin/MSL_C/PPCEABI/bare/H/ansi_fp.c",
                mw_version="GC/1.3",
                extra_cflags=[
                    "-inline",
                    "all",
                    "-inline",
                    "auto,deferred",
                    "-use_lmw_stmw",
                    "on",
                    "-char",
                    "signed",
                    "-str",
                    "pool,readonly",
                ],
            ),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/buffer_io.c", mw_version="GC/1.3"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/direct_io.c", mw_version="GC/1.3", extra_cflags=["-use_lmw_stmw", "on"]),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/file_io.c", mw_version="GC/1.3"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/FILE_POS.c", mw_version="GC/1.3"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/mbstring.c", mw_version="GC/1.3.2r", cflags=cflags_msl),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/mem.c", mw_version="GC/1.3"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/mem_funcs.c", mw_version="GC/1.3"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/misc_io.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/printf.c", mw_version="GC/1.3", extra_cflags=["-use_lmw_stmw", "on", "-char", "signed"]),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/string.c", mw_version="GC/1.3"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/wchar_io.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/ctype.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/s_copysign.c", mw_version="GC/1.3"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/s_frexp.c", mw_version="GC/1.3"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/s_ldexp.c", mw_version="GC/1.3"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/s_modf.c", mw_version="GC/1.3"),
            Object(MatchingFor("GSAE01"), "dolphin/base/PPCArch_weak.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/ctype_funcs.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/uart_console_io_gcn.c", mw_version="GC/1.2.5"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/hyperbolicsf.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/floorf.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/rand.c", mw_version="GC/1.1", extra_cflags=["-O0"]),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/math_ppc.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/s_cos.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/s_atan.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/e_acos.c"),
            Object(NonMatching, "dolphin/MSL_C/PPCEABI/bare/H/e_fmod.c"),
            Object(
                NonMatching,
                "dolphin/MSL_C/PPCEABI/bare/H/exponentialsf.c",
                mw_version="GC/1.1",
                extra_cflags=["-O3,p", "-opt", "nopeephole", "-sdata", "0"],
            ),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/extras.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/k_rem_pio2.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/w_acos.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/w_atan2.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/w_fmod.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/w_pow.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/w_sqrt.c"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/common_float_tables.c", mw_version="GC/1.3"),
            Object(MatchingFor("GSAE01"), "dolphin/MSL_C/PPCEABI/bare/H/trigf.c", mw_version="GC/1.2.5"),
        ],
    ),
    {
        "lib": "main",
        "mw_version": "GC/2.0",
        "cflags": cflags_base,
        "progress_category": "game",
        "objects": [
            # dlls/engine

            # dlls/modgfx

            # dlls/projgfx

            # dlls/objects
            Object(
                NonMatching,
                "dlls/objects/195_Player/player.c",
                mw_version="GC/1.3",
                cflags=[*cflags_dll_noopt, "-inline", "noauto"],
            ),
            Object(
                NonMatching,
                "dlls/objects/196_Tricky/tricky.c",
                mw_version="GC/2.0",
                cflags=[
                    *cflags_base,
                    "-opt",
                    "nopeephole,noschedule,nopropagation,nodead",
                    "-inline",
                    "noauto",
                ],
            ),
            Object(MatchingFor("GSAE01"), "dlls/objects/197/197.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/198_AnimatedObj/AnimatedObj.c", cflags=cflags_dll_noopt),
            Object(
                MatchingFor("GSAE01"),
                "dlls/objects/199_DIM2RoofRub/DIM2RoofRub.c",
                cflags=cflags_dll_noopt_noprop,
            ),
            Object(
                MatchingFor("GSAE01"),
                "dlls/objects/200_DepthOfFieldPoint/DepthOfFieldPoint.c",
                cflags=cflags_dll_noopt,
            ),
            Object(NonMatching, "dlls/objects/201_Baddie/Baddie.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/202/202.c", cflags=cflags_dll_noopt_noautoinline),
            Object(Matching, "dlls/objects/203/203.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/204_ChukChuk/ChukChuk.c", cflags=cflags_dll_noopt_noprop_noinline),
            Object(Matching, "dlls/objects/205_IceBall/IceBall.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/206/206.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/207_CannonClaw/CannonClaw.c", cflags=cflags_dll_noopt),
            Object(Matching, "dlls/objects/208_Grimble/Grimble.c", cflags=cflags_dll_noopt),

            # main
            Object(MatchingFor("GSAE01"), "main/audio/synth_constants.c"),
            Object(MatchingFor("GSAE01"), "main/audio/synth_callback.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/synth_channel.c", mw_version="GC/1.2.5n"),
            Object(MatchingFor("GSAE01"), "main/audio/synth_handle.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/synth_sequence.c", mw_version="GC/1.2.5n"),
            Object(MatchingFor("GSAE01"), "main/audio/synth_seq_queue.c", mw_version="GC/1.2.5n"),
            Object(MatchingFor("GSAE01"), "main/audio/synth_control.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/snd_synth_api.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/mcmd_volume.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/vid_init.c", mw_version="GC/1.2.5n"),
            Object(MatchingFor("GSAE01"), "main/audio/synth_vsamples.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(NonMatching, "main/audio/voice.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/voice_conv.c", mw_version="GC/1.2.5n"),
            Object(MatchingFor("GSAE01"), "main/audio/synth_ac.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/synth_adsr.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(NonMatching, "main/render.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/audio.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/audio_sfx.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/audio_stream.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/camera.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/curves.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/voxmaps.c", cflags=cflags_dll_noopt_noautoinline, mw_version="GC/1.3"),
            Object(Matching, "main/modelEngine.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/pad.c", cflags=cflags_dll_noopt_nocse, mw_version="GC/1.3"),
            Object(Matching, "main/fileio.c", cflags=cflags_dll_noopt_noloopinv_noautoinline),
            Object(NonMatching, "main/gametext.c", cflags=cflags_dll_noopt_nolifetimes_noinline),
            Object(MatchingFor("GSAE01"), "main/gametext_measurebyid.c", cflags=cflags_dll_noopt_nocse_noinline),
            Object(NonMatching, "main/gametext_tail.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/textrender.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/textrender_gettext.c", cflags=cflags_dll_noopt_noprop),
            Object(NonMatching, "main/textrender_run.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/textrender_subtitle.c", cflags=cflags_dll_noopt_level1),
            Object(NonMatching, "main/textrender_drawbox.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/textrender_boxtex.c", cflags=cflags_dll_noopt_nocse_nolifetimes_noloopinv_noprop_nostrength),
            Object(NonMatching, "main/modellight.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/gameloop.c", cflags=[*cflags_dll_noopt, "-inline", "noauto"]),
            Object(NonMatching, "main/gameloop_buttonobj.c", cflags=[*cflags_dll_nosched, "-inline", "noauto"]),
            Object(NonMatching, "main/gameloop_main.c", cflags=[*cflags_dll_noopt, "-inline", "noauto"]),
            Object(NonMatching, "main/vecmath.c", cflags=cflags_dll_noopt_nostrength),
            Object(NonMatching, "main/mm.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/model.c", cflags=cflags_dll_noopt_noloopinv),
            Object(NonMatching, "main/object.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/sky.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/sky2.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/newclouds.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/skystars.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/cloudaction.c", cflags=cflags_dll_noopt, mw_version="GC/1.3"),
            Object(
                NonMatching, "main/dll/dll_0013_waterfx.c", cflags=cflags_dll_noopt_noautoinline,
                section_alignments={".data": 4},
            ),
            Object(MatchingFor("GSAE01"), "main/audio_decode_thread.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/523_FireFly/FireFly.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/524_WM_spiritpl/WM_spiritpl.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/525_WM_seqpoint/WM_seqpoint.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/526_WM_sun/WM_sun.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/527_WM_SpiritSe/WM_SpiritSe.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/528_WM_Planets/WM_Planets.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/529/529.c", cflags=cflags_dll_noopt_noloopinv_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/530/530.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/531_WM_VConsole/WM_VConsole.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/532_WM_TransTop/WM_TransTop.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/533_WM_newcryst/WM_newcryst.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/534_VFP_LevelCo/VFP_LevelCo.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/535_VFP_ObjCrea/VFP_ObjCrea.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/536_VFP_MiniFir/VFP_MiniFir.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/537/537.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/538_VFP_statueb/VFP_statueb.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/539/539.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/540_VFP_Ladders/VFP_Ladders.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/541/541.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/542_VFP_Block1/VFP_Block1.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/543/543.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/544/544.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/545/545.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "dlls/objects/546_VFPDragHead/VFPDragHead.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/547_VFP_corepla/VFP_corepla.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/548/548.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/549_VFP_flamepo/VFP_flamepo.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/550_VFP_lavapoo/VFP_lavapoo.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/551_VFP_lavasta/VFP_lavasta.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/552/552.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/553_DFP_LevelCo/DFP_LevelCo.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/554_DFP_ObjCrea/DFP_ObjCrea.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/555_DFP_Torch/DFP_Torch.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "dlls/objects/556/556.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/557_DFP_seqpoin/DFP_seqpoin.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/558/558.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/559_DFP_floorba/DFP_floorba.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/560_DFP_wallbar/DFP_wallbar.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/561_DFP_ForceAw/DFP_ForceAw.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/562_DFP_RotateP/DFP_RotateP.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/563_DFP_Statue1/DFP_Statue1.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/564_DFP_PerchSw/DFP_PerchSw.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/565_DFP_TargetB/DFP_TargetB.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/566_DFP_LaserBe/DFP_LaserBe.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/567_DFPSpPl/DFPSpPl.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/568_LINKA_levco/LINKA_levco.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/569/569.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/570_DFP_Platfor/DFP_Platfor.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/571_DFP_Lightni/DFP_Lightni.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/572_DFP_PowerSl/DFP_PowerSl.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/573_DBPointMum/DBPointMum.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/574/574.c", cflags=cflags_dll_noopt),
            Object(Matching, "dlls/objects/575_DB_egg/DB_egg.c", cflags=cflags_dll_noopt_noloopinv),
            Object(MatchingFor("GSAE01"), "dlls/objects/576_GCRobotBlas/GCRobotBlas.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/577_DrakorEnerg/DrakorEnerg.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/578_DBstealerwo/DBstealerwo.c", cflags=cflags_dll_noopt_noloopinv_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/579_DBHoleContr/DBHoleContr.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/580/580.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/581/581.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/582/582.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/583/583.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/584/584.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/585/585.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/586/586.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/587/587.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/588_BossDrakor_/BossDrakor_.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/589_BossDrakor/BossDrakor.c", cflags=cflags_dll_noopt_nocse_noprop_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/590/590.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "dlls/objects/591_KT_RexLevel/KT_RexLevel.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/592_KT_Rex/KT_Rex.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/593_KT_RexFloor/KT_RexFloor.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/594_KT_Lazerwal/KT_Lazerwal.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/595_KT_Lazerlig/KT_Lazerlig.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/596_KT_Fallingr/KT_Fallingr.c", cflags=cflags_dll_noopt),
            Object(
                NonMatching, "dlls/objects/597/597.c", cflags=cflags_dll_noopt_noautoinline,
                section_alignments={".data": 4},
            ),
            Object(MatchingFor("GSAE01"), "main/audio/snd_midictrl.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/snd_service.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/hw_init.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/hw_adsr.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/hw_voice_params.c", mw_version="GC/1.2.5n"),
            Object(MatchingFor("GSAE01"), "main/audio/hw_voice_start.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/hw_input.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/hw_aram.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/hw_samplemem.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(NonMatching, "dlls/objects/437/437.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/598_DIMSnowHorn/DIMSnowHorn.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "dlls/objects/599_DR_EarthWar/DR_EarthWar.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/600_DR_CloudRun/DR_CloudRun.c", cflags=cflags_dll_noopt_noloopinv_noautoinline),
            Object(NonMatching, "main/objanim.c", cflags=cflags_dll_noopt_nocse),
            Object(NonMatching, "main/lightmap.c", cflags=cflags_dll_noopt_noautoinline_deferred),
            Object(NonMatching, "main/lightmap_initmapblocks.c", cflags=cflags_dll_noopt_noautoinline_deferred),
            Object(
                NonMatching, "main/lightmap_draw.c", cflags=cflags_dll_noopt_noautoinline_deferred,
                section_alignments={".data": 4},
            ),
            Object(Matching, "main/objHitReact.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/objhits.c", cflags=cflags_dll_noopt_noautoinline_level3),
            Object(NonMatching, "main/objlib.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/objprint.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/objprint_dolphin.c", cflags=[*cflags_dll_noopt_noloopinv_nolifetimes_zerodata, "-inline", "noauto"]),
            Object(NonMatching, "main/pi_dolphin.c", cflags=[*cflags_dll_noopt_noloopinv_zerodata, "-inline", "noauto"]),
            Object(NonMatching, "main/pi_videoinit.c", cflags=[*cflags_dll_noopt_noloopinv_noprop_zerodata, "-inline", "noauto"]),
            Object(NonMatching, "main/pi_pathsearch.c", cflags=[*cflags_dll_noopt_noloopinv_zerodata, "-inline", "noauto"]),
            Object(NonMatching, "main/zlb.c", **zlb_object_kwargs),
            Object(NonMatching, "main/shader_dolphin.c", cflags=cflags_dll_noopt_noloopinv_noautoinline),
            Object(MatchingFor("GSAE01"), "main/boot_logo.c"),
            Object(NonMatching, "main/rcp_dolphin.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/texture.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/shader.c", cflags=cflags_dll_noopt_noautoinline_deferred),
            Object(
                NonMatching, "main/tex_dolphin.c", cflags=cflags_dll_noopt_noautoinline_deferred,
                section_alignments={".data": 4},
            ),
            Object(NonMatching, "main/shadow_dolphin.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/track_dolphin.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/newshadows.c", cflags=cflags_dll_noopt_noautoinline),
            Object(
                NonMatching, "track/intersect_surfacesfx.c", cflags=cflags_dll_noopt_nocse_noautoinline,
                section_alignments={".data": 4},
            ),
            Object(
                NonMatching, "track/intersect.c", cflags=cflags_dll_noopt_noautoinline,
                section_alignments={".data": 4},
            ),
            Object(MatchingFor("GSAE01"), "track/intersect_earthwalkersfx.c", cflags=cflags_dll_noopt_nocse_noautoinline),
            Object(NonMatching, "track/intersect_render.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "track/intersect_texsetup.c", cflags=cflags_dll_noopt_nocse_noautoinline),
            Object(NonMatching, "track/intersect_memcard.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/maketex.c", cflags=cflags_dll_noopt_noloopinv_noautoinline),
            Object(NonMatching, "main/dll/expgfx.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/dll/dll_000B_dll0b.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/dll_000C_projgfx.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_000D_playershadow.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_0018_boneparticleeffect.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_000E_partfx.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_001A_effect1.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_001B_effect2.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_001C_effect3.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_001D_effect4.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_001E_effect5.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_001F_effect6.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0020_effect7.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0021_effect8.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0022_effect9.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0023_effect10.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0024_effect11.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0025_effect12.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0027_effect14.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0029_effect16.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0028_effect15.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0026_effect13.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_002A_effect17.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_002B_effect18.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_002C_effect19.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_002D_effect20.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_0003_checkpoint.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/dll/dll_0016_screentransition.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0004_dummy04.c"),
            Object(MatchingFor("GSAE01"), "main/dll/dll_000F_unk.c", cflags=cflags_dll_noopt_nocse_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0010_uicontroller.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0012_unk.c"),
            Object(NonMatching, "main/dll/Hcurves.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/dll/Hcurves_romcurve.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/dll/dll_0015_curves.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/dll/dll_0017_savegame.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0011_screens.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_002F_carryable.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0059_dll59func0.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_005A_staffcollisionfunc03.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_005B_modgfxfunc03.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_005C_dll5cfunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_005D_dll5dfunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_005E_dll5efunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_005F_dll5ffunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0060_dll60func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0061_dll61func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0062_dll62func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0063_dll63func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0064_dll64func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0065_dll65func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00A3_dlla3func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0066_dll66func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0067_dll67func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0068_dll68func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0058_dummy58.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0069_dll69func0.c", cflags=cflags_dll_noopt_noprop),
            Object(MatchingFor("GSAE01"), "main/dll/dll_006A_dll6afunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_006B_dll6bfunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_006C_dummy6c.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_006D_dll6dfunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_006E_dll6efunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_006F_dll6ffunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0070_dll70func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0071_dll71func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0072_dll72func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0073_dll73func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0074_dll74func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0075_dll75func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0076_dll76func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0077_dll77func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0078_dll78func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0079_dll79func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_007A_dll7afunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_007B_dll7bfunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_007C_dll7cfunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_007D_dll7dfunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_007E_dll7efunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_007F_dll7ffunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0080_dll80func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0081_dll81func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0082_dll82func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0083_dll83func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0084_dll84func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0085_dll85func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0086_dll86func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0087_dll87func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0088_dll88func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0089_dll89func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_008A_dll8afunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_008B_dll8bfunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_008C_dll8cfunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_008D_dll8dfunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_008E_dll8efunc0.c", cflags=cflags_dll_noopt_noprop),
            Object(MatchingFor("GSAE01"), "main/dll/dll_008F_dll8ffunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0090_dll90func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0091_dll91func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0092_dll92func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0093_dll93func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0094_dll94func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0095_dll95func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0096_dll96func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0097_dll97func0.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_0098_dll98func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0099_dll99func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_009A_dll9afunc0.c", cflags=cflags_dll_noopt_noprop),
            Object(MatchingFor("GSAE01"), "main/dll/dll_009B_dll9bfunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_009C_dll9cfunc0.c", cflags=cflags_dll_noopt, extra_cflags=["-opt", "level=3,nopropagation"]),
            Object(MatchingFor("GSAE01"), "main/dll/dll_009D_dll9dfunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_009E_dll9efunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_009F_dll9ffunc0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00A0_dlla0func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00A1_dlla1func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00A2_dlla2func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00A4_dummya4.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00A5_dlla5func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00A6_dlla6func0.c", cflags=cflags_dll_noopt_noprop),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00A7_dlla7func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00A8_dlla8func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00A9_dlla9func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00AA_dllaafunc0.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/cutcam.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/dll/dll_0042_unk.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/pathcamgroup.c", cflags=cflags_dll_noopt),
            Object(
                NonMatching, "main/dll/dll_0045_camTalk.c", cflags=cflags_dll_noopt_noprop,
                section_alignments={".sdata2": 4},
            ),
            Object(NonMatching, "main/dll/dll_0044_cameramodeviewfinder.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0046_cameramodedebug.c", cflags=cflags_dll_noopt_nocse_noprop),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0048_cameramodestatic.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "main/dll/dll_8010a104.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_004B_cameramodeclimb.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_002E_moveLib.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_b8.c", cflags=cflags_dll_noopt),
            Object(Matching, "main/dll/n_attractmode.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0034_titlemenu.c", cflags=cflags_dll_noopt),
            Object(Matching, "main/dll/modanimeflash1.c"),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00AB_projdummy.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00AC_projmagicstream.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00AD_projmagicemmit1.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00AE_projroombeam.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00AF_projlightning1.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00B0_projlightning2.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00B1_projlightning3.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00B3_projlightning4.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00B8_projlightning5.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00B9_projlightning7.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00BA_projlightning6.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00B4_projenergise1.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00B5_projenergise2.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00B2_projrobotfire.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00B6_projsquirt1.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00B7_projship1.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00BB_projwallpower.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00BC_projquakeshock.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00BD_projsunshock.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00BE_projtesla.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00BF_projcore1.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00C0_projcore2.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00C1_projcore3.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_00C2_projdfp1r.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_8b.c"),
            Object(Matching, "main/dll/dll_8c.c"),
            Object(Matching, "main/dll/dll_8d.c"),
            Object(Matching, "main/dll/dll_8f.c"),
            Object(Matching, "main/dll/dll_90.c"),
            Object(Matching, "main/dll/dll_92.c"),
            Object(Matching, "main/dll/dll_93.c"),
            Object(Matching, "main/dll/dll_95.c"),
            Object(Matching, "main/dll/dll_96.c"),
            Object(Matching, "main/dll/dll_98.c"),
            Object(Matching, "main/dll/dll_99.c"),
            Object(Matching, "main/dll/dll_9b.c"),
            Object(Matching, "main/dll/dll_9c.c"),
            Object(Matching, "main/dll/dll_9e.c"),
            Object(Matching, "main/dll/dll_9f.c"),
            Object(Matching, "main/dll/dll_a1.c"),
            Object(Matching, "main/dll/dll_a2.c"),
            Object(Matching, "main/dll/dll_a4.c"),
            Object(Matching, "main/dll/dll_a5.c"),
            Object(MatchingFor("GSAE01"), "main/dll/dll_a6.c", cflags=cflags_dll_noopt),
            Object(Matching, "main/dll/dll_a7.c"),
            Object(Matching, "main/dll/dll_a8.c"),
            Object(Matching, "main/dll/dll_aa.c"),
            Object(Matching, "main/dll/dll_ab.c"),
            Object(Matching, "main/dll/dll_ad.c"),
            Object(Matching, "main/dll/dll_ae.c"),
            Object(Matching, "main/dll/dll_b1.c"),
            Object(MatchingFor("GSAE01"), "main/dll/dll_b2.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_b3.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_b4.c", cflags=cflags_dll_nosched),
            Object(NonMatching, "main/dll/dll_b6.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_b7.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_bb.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_bc.c", cflags=cflags_dll_nosched),
            Object(Matching, "main/dll/dll_bd.c"),
            Object(Matching, "main/dll/dll_bf.c"),
            Object(Matching, "main/dll/dll_c4.c"),
            Object(
                NonMatching, "main/dll/CAM/camcontrol.c", cflags=cflags_dll_noopt,
                section_alignments={".data": 4},
            ),
            Object(MatchingFor("GSAE01"), "main/dll/CAM/cammodes.c", cflags=cflags_dll_noopt),
            Object(Matching, "main/dll/dll_53.c"),
            Object(
                NonMatching, "main/dll/camshipbattle5c.c", cflags=cflags_dll_noopt_nocse,
                section_alignments={".data": 4},
            ),
            Object(NonMatching, "main/dll/dll_0047_cameramodeteststrength.c", cflags=cflags_dll_noopt_noautoinline),
            Object(Matching, "main/dll/attentioncam.c"),
            Object(MatchingFor("GSAE01"), "main/dll/dll_60.c"),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0049_cameramodecombat_intro.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0049_cameramodecombat.c", cflags=cflags_dll_noopt_nocse),
            Object(NonMatching, "main/dll/dll_004A_cameramodeshipbattle.c", cflags=cflags_dll_noopt_nocse_noprop),
            Object(MatchingFor("GSAE01"), "main/dll/dll_004C_camDebug.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_004D_cameramodenpcspeak.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/dll/dll_004E_cameramodeworldmap.c", cflags=cflags_dll_noopt_nocse_noprop),
            Object(MatchingFor("GSAE01"), "main/dll/dll_004F_dll4f.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0050_cameramodecrawl.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0051_cameramodecannon.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0052_cameramodeforcebehind.c", cflags=cflags_dll_noopt_noprop),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0053_cameramodecloudrunner.c", cflags=cflags_dll_noopt_noprop),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0054_dll54.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0055_cameramodeperv.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_0056_cameramodearwing.c", cflags=cflags_dll_noopt_nocse_noprop),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0057_cameramodetitle.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_0019_dll19func0.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0032_titlescreeninit.c", cflags=cflags_dll_noopt_nocse_noprop),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0033_nrareware.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_3b.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "main/dll/n_options.c", cflags=cflags_dll_noopt),
            Object(Matching, "main/dll/n_pausemenu.c"),
            Object(
                MatchingFor("GSAE01"), "main/dll/dll_3e.c", cflags=cflags_dll_noopt,
                section_alignments={".sbss": 4},
            ),
            Object(Matching, "main/dll/dll_40.c"),
            Object(MatchingFor("GSAE01"), "main/dll/attractmovie.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/picmenu.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "main/dll/frontend_control.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_43.c", cflags=cflags_dll_noopt),
            Object(
                NonMatching, "main/dll/dll_0035_saveselectscreen.c", cflags=cflags_dll_noopt_noinline,
                section_alignments={".data": 4},
            ),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0036_entersavenamescreen.c", cflags=cflags_dll_noopt, mw_version="GC/1.3"),
            Object(Matching, "main/dll/dll_48.c"),
            Object(Matching, "main/dll/dll_49.c"),
            Object(Matching, "main/dll/dll_4b.c"),
            Object(Matching, "main/dll/swaphol.c"),
            Object(MatchingFor("GSAE01"), "main/dll/dll_4e.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_4d.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/prof.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0037_optionsscreen.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0038_weirdunusedmenu.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0039_dummy39.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_003A_dummy3a.c"),
            Object(MatchingFor("GSAE01"), "main/dll/dll_8011d918.c", cflags=cflags_dll_nosched),
            Object(NonMatching, "main/dll/dll_0000_gameui.c", cflags=cflags_dll_noopt, extra_cflags=["-inline", "noauto"]),
            Object(MatchingFor("GSAE01"), "main/dll/dll_003B_menu.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_003C_tumbleweedbush.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "main/dll/dll_003D_titlemenuitem.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_003E_dummy3e.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_0031_minimap.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/dll_003F_dll3f.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0040_credits.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_0041_warpstoneui.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_02C0_front.c", cflags=cflags_dll_noopt_noloopinv),
            Object(NonMatching, "main/dll/dll_80136a40.c", cflags=cflags_dll_noopt_nostrength),
            Object(MatchingFor("GSAE01"), "dlls/objects/223_Hagabon/Hagabon.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/224_SwarmBaddie/SwarmBaddie.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/225_WispBaddie/WispBaddie.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/battledroid.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/newseqobjgroup.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/groundbaddiepush.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/seqobj11e.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/gcrobotpatrol.c", cflags=cflags_dll_noopt_nocse_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/mikaladon.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/magicplant.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/kooshy.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "main/dll/weevil.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/fallladdersgroup.c", cflags=cflags_dll_noopt_noprop_noinline),
            Object(MatchingFor("GSAE01"), "main/dll/fireflylantern_steer.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/duster.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/duster_80155770.c", cflags=cflags_dll_noopt_nocse_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/spittingeba.c", cflags=cflags_dll_noopt_noautoinline),
            Object(Matching, "main/dll/duster_wb.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/hoodedzyck.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/firecrawler.c", cflags=cflags_dll_noopt_noloopinv_noinline),
            Object(MatchingFor("GSAE01"), "main/dll/hagabon_mk2.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/snowworm.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "main/dll/baddiewhirlpool.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/209_TumbleWeedB/TumbleWeedB.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "dlls/objects/210/210.c", cflags=cflags_dll_noopt_noloopinv_noautoinline),
            Object(NonMatching, "dlls/objects/211/211.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/212_SkeetlaWall/SkeetlaWall.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/213_Kaldachom/Kaldachom.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/214_KaldachomMe/KaldachomMe.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/215/215.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/216_PinPonSpike/PinPonSpike.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/217_Pollen/Pollen.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/218/218.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/219_MikaBomb/MikaBomb.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/220_MikaBombSha/MikaBombSha.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/602_StaticCamer/StaticCamer.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/221_GCbaddieShi/GCbaddieShi.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/222_baddieInter/baddieInter.c", cflags=cflags_dll_noopt_noloopinv),
            Object(
                NonMatching, "dlls/objects/226/226.c", cflags=cflags_dll_noopt,
                section_alignments={".data": 4},
            ),
            Object(
                MatchingFor("GSAE01"),
                "dlls/objects/227/227.c",
                cflags=cflags_dll_noopt,
                section_alignments={".data": 4},
            ),
            Object(
                MatchingFor("GSAE01"),
                "dlls/objects/228/228.c",
                cflags=cflags_dll_noopt_nocse,
            ),
            Object(NonMatching, "dlls/objects/229/229.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/293_curve/curve.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/230_ReStartMark/ReStartMark.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/247/247.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/232_Checkpoint4/Checkpoint4.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/234_Sideload/Sideload.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/235/235.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/233_Setuppoint/Setuppoint.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/237/237.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/255/255.c", cflags=cflags_dll_noopt_noloopinv),
            Object(MatchingFor("GSAE01"), "dlls/objects/238_EffectBox/EffectBox.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/239/239.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/240_WarpPoint/WarpPoint.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/241_InvHit/InvHit.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/242_iceblast/iceblast.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "dlls/objects/243_flameblast/flameblast.c", cflags=cflags_dll_noopt_nocse_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/244/244.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/245_SidekickBal/SidekickBal.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/246_Area/Area.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/248_LevelName/LevelName.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/249/249.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/250_InvisibleHi/InvisibleHi.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/251/251.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/272/272.c", cflags=cflags_dll_noopt_noloopinv),
            Object(MatchingFor("GSAE01"), "dlls/objects/271_MMP_Bridge/MMP_Bridge.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/273/273.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/274/274.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/275/275.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/276_IMMultiSeq/IMMultiSeq.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/277/277.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/278_WM_Column/WM_Column.c", cflags=cflags_dll_noopt),
            Object(
                NonMatching,
                "dlls/objects/279_AppleOnTree/AppleOnTree.c",
                cflags=cflags_dll_noopt_noautoinline,
            ),
            Object(MatchingFor("GSAE01"), "dlls/objects/252/252.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/253/253.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/254_MagicPlant/MagicPlant.c", cflags=cflags_dll_noopt_nocse_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/256_TrickyWarp/TrickyWarp.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/257_TrickyGuard/TrickyGuard.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/258_StayPoint/StayPoint.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/280_Duster/Duster.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/259_CurveFish/CurveFish.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/260_SmallBasket/SmallBasket.c", cflags=cflags_dll_noopt_noprop),
            Object(MatchingFor("GSAE01"), "dlls/objects/261_LargeCrate/LargeCrate.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/262/262.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/263/263.c", cflags=cflags_dll_noopt_nocse_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/264_EndObject/EndObject.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/269_PortalSpell/PortalSpell.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/268_LanternFire/LanternFire.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/267_FireFlyLant/FireFlyLant.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/231/231.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/265/265.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/266_Fall_Ladder/Fall_Ladder.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/281_coldWaterCo/coldWaterCo.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/236_InfoPoint/InfoPoint.c", cflags=cflags_dll_noopt),
            Object(Matching, "dlls/objects/282/282.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/283_Landed_Arwi/Landed_Arwi.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/284/284.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/285/285.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/286_MagicCaveBo/MagicCaveBo.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/287_MagicCaveTo/MagicCaveTo.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/288_TrickyGuard/TrickyGuard.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/289/289.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/290_CCTestInfot/CCTestInfot.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/292/292.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/291_fuelCell/fuelCell.c", cflags=cflags_dll_noopt_noloopinv),
            Object(MatchingFor("GSAE01"), "dlls/objects/270/270.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/295/295.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/297_CampFire/CampFire.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/296_KT_Torch/KT_Torch.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/298_CFCrate/CFCrate.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/299_FXEmit/FXEmit.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/304_AreaFXEmit/AreaFXEmit.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/301_LFXEmitter/LFXEmitter.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/warppad.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "dlls/objects/300_Transporter/Transporter.c", cflags=cflags_dll_noopt_noloopinv),
            Object(MatchingFor("GSAE01"), "dlls/objects/302/302.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/303_BarrelPad/BarrelPad.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/305/305.c", cflags=cflags_dll_noopt),
            Object(Matching, "main/dll/holopoint.c"),
            Object(MatchingFor("GSAE01"), "dlls/objects/308_texscroll2/texscroll2.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/309_texscroll/texscroll.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/310_WaveAnimato/WaveAnimato.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/311_AlphaAnimat/AlphaAnimat.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/312_GroundAnima/GroundAnima.c", cflags=cflags_dll_noopt),
            Object(Matching, "dlls/objects/313_HitAnimator/HitAnimator.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/314_VisAnimator/VisAnimator.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/315_WallAnimato/WallAnimato.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/316_XYZAnimator/XYZAnimator.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/317_ExplodeAnim/ExplodeAnim.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/318/318.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/341/341.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/319_TexFrameAni/TexFrameAni.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/320_fogControl/fogControl.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/321_Lightning/Lightning.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/306_WaterFallSp/WaterFallSp.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/307_sfxPlayer/sfxPlayer.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_80198a00.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/mmp_gyservent.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/294/294.c", cflags=cflags_dll_noopt_noloopinv_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/325_CloudPrison/CloudPrison.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/326_CloudShipCo/CloudShipCo.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/327/327.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/328_CFGuardian/CFGuardian.c", cflags=cflags_dll_noopt_nocse_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/329/329.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/330_CFPowerBase/CFPowerBase.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/331_CFMainCryst/CFMainCryst.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/332/332.c", cflags=cflags_dll_noopt_nocse_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/333_LaserBeam/LaserBeam.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/334_CFPrisonGua/CFPrisonGua.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/335_CFPrisonUnc/CFPrisonUnc.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/336_GCRobotLigh/GCRobotLigh.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/337_CFScalesGal/CFScalesGal.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/338_CF_ObjCreat/CF_ObjCreat.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/339_CFPerch/CFPerch.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/340/340.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/342/342.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/343_SpiritDoorS/SpiritDoorS.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "dlls/objects/344/344.c", cflags=cflags_dll_noopt_noinline),
            Object(Matching, "dlls/objects/345/345.c", cflags=cflags_dll_noopt_noloopinv_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/346/346.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/347_CFForceFiel/CFForceFiel.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/348_CFForceFiel/CFForceFiel.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/349/349.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/350/350.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/353_CFTreasRobo/CFTreasRobo.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/351/351.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/352/352.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/355/355.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/357_CFRemovalSh/CFRemovalSh.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/354_CFMagicWall/CFMagicWall.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/356_CFLevelCont/CFLevelCont.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/358/358.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/359_SpiritDoorL/SpiritDoorL.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/360_HoloPoint/HoloPoint.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/380_GCRobotPatr/GCRobotPatr.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/381/381.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/382_MMP_levelco/MMP_levelco.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/383/383.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/384_MMP_asteroi/MMP_asteroi.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/386_MMP_moonroc/MMP_moonroc.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/385_MMP_trenchF/MMP_trenchF.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/387_MMP_gyserve/MMP_gyserve.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/388/388.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/603_MSPlantingS/MSPlantingS.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/389_CCgasvent/CCgasvent.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/390_CCgasventCo/CCgasventCo.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/391_CCqueen/CCqueen.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/392_CClightfoot/CClightfoot.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/393_CCSharpclaw/CCSharpclaw.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/394_CCpedstal/CCpedstal.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/395_CClevcontro/CClevcontro.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/361_IMIceMounta/IMIceMounta.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/362_CRrockfall/CRrockfall.c", cflags=cflags_dll_noopt_noprop_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/363/363.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/364/364.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/365_IMIcePillar/IMIcePillar.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/366_IMAnimSpace/IMAnimSpace.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/367_IMSpaceThru/IMSpaceThru.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/368_IMSpaceRing/IMSpaceRing.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/369_IMSpaceRing/IMSpaceRing.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/370_LINKB_levco/LINKB_levco.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/371_LINK_levcon/LINK_levcon.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/446/446.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/447_DIMLavaBall/DIMLavaBall.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/448_DIMLogFire/DIMLogFire.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/449_DIMSnowBall/DIMSnowBall.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/DIM/dimwooddoor.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/454_DIMCannon/DIMCannon.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/455_DIMLavaSmas/DIMLavaSmas.c", cflags=cflags_dll_noopt_noprop_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/456_DIMBridgeCo/DIMBridgeCo.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/457_DIMDismount/DIMDismount.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/458_DIMExplosio/DIMExplosio.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/459_DIMWoodDoor/DIMWoodDoor.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/462/462.c", cflags=cflags_dll_noopt_nostrength),
            Object(MatchingFor("GSAE01"), "dlls/objects/460_DIMMagicBri/DIMMagicBri.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/461_DIM_LevelCo/DIM_LevelCo.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/463/463.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/464_DIM_tricky/DIM_tricky.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/465_DIMTruthHor/DIMTruthHor.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/466_WORLDplanet/WORLDplanet.c", cflags=cflags_dll_noopt, mw_version="GC/1.3"),
            Object(NonMatching, "dlls/objects/467/467.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/468_WORLDAstero/WORLDAstero.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/469_DIM2Conveyo/DIM2Conveyo.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/470/470.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "dlls/objects/471_DIM2SnowBal/DIM2SnowBal.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/472_DIM2PathGen/DIM2PathGen.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/473_DIM2PrisonM/DIM2PrisonM.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/474/474.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/475/475.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/476_DIM2IceFloe/DIM2IceFloe.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/477_DIM2Icicle/DIM2Icicle.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/478_DIM2LavaCon/DIM2LavaCon.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "dlls/objects/479/479.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/480_DIM_Boss/DIM_Boss.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/481_DIM_BossGut/DIM_BossGut.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/482_DIM_BossTon/DIM_BossTon.c", cflags=cflags_dll_noopt_noprop),
            Object(MatchingFor("GSAE01"), "dlls/objects/483_DIM_BossGut/DIM_BossGut.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/485_DIM_BossSpi/DIM_BossSpi.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/484_MAGICMaker/MAGICMaker.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/486_DIMbosscrac/DIMbosscrac.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/487_DIMbossfire/DIMbossfire.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/372_CCriverflow/CCriverflow.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/373_DFropenode/DFropenode.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/374_DFSH_Door1S/DFSH_Door1S.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/375/375.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/376_DFSH_Shrine/DFSH_Shrine.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/378_SpiritPrize/SpiritPrize.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/377_DFSH_ObjCre/DFSH_ObjCre.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/379_DFSH_LaserB/DFSH_LaserB.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/396_MMSH_Shrine/MMSH_Shrine.c", cflags=cflags_dll_noopt_nolifetimes_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/397_MMSH_Scales/MMSH_Scales.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/398_MMSH_WaterS/MMSH_WaterS.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/399_ECSH_Shrine/ECSH_Shrine.c", cflags=cflags_dll_noopt_nostrength),
            Object(MatchingFor("GSAE01"), "dlls/objects/401_ECSH_Creato/ECSH_Creato.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/402_GPSH_Shrine/GPSH_Shrine.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/403_GPSH_ObjCre/GPSH_ObjCre.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/404_GPSH_Scene/GPSH_Scene.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/400_ECSH_Cup/ECSH_Cup.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/405_DBSH_Shrine/DBSH_Shrine.c", cflags=cflags_dll_noopt),
            Object(Matching, "main/dll/colrise.c"),
            Object(MatchingFor("GSAE01"), "dlls/objects/406_DBSH_Symbol/DBSH_Symbol.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/407/407.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/408_NWSH_levcon/NWSH_levcon.c", cflags=cflags_dll_noopt),
            Object(Matching, "main/dll/symbol.c"),
            Object(MatchingFor("GSAE01"), "dlls/objects/409/409.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/410/410.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/411/411.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/412/412.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/413/413.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/414/414.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/415_NW_treebrid/NW_treebrid.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/416_NW_geyser/NW_geyser.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/417/417.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/418_NW_tricky/NW_tricky.c", cflags=cflags_dll_noopt_noloopinv),
            Object(MatchingFor("GSAE01"), "dlls/objects/419/419.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/420/420.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/421_NW_levcontr/NW_levcontr.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/422_SH_tricky/SH_tricky.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/423/423.c", cflags=cflags_dll_noopt_noloopinv_nolifetimes_nodead),
            Object(MatchingFor("GSAE01"), "dlls/objects/424_SH_killermu/SH_killermu.c", cflags=cflags_dll_noopt_nocse_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/425_BombPlant/BombPlant.c", cflags=cflags_dll_noopt_noautoinline),
            Object(Matching, "dlls/objects/426_BombPlantSp/BombPlantSp.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/427_BombPlantin/BombPlantin.c", cflags=cflags_dll_noopt),
            Object(Matching, "dlls/objects/428_SH_queenear/SH_queenear.c", cflags=cflags_dll_noopt_noinline),
            Object(Matching, "main/dll/lily.c"),
            Object(Matching, "main/dll/dll_1e8.c"),
            Object(NonMatching, "dlls/objects/429_SH_thorntai/SH_thorntai.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/430_SH_LevelCon/SH_LevelCon.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/431_SH_swaplift/SH_swaplift.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/432_SH_swapston/SH_swapston.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/433_SH_staff/SH_staff.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/434_SH_staffHaz/SH_staffHaz.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/435_SH_Beacon/SH_Beacon.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/436_SH_EmptyTum/SH_EmptyTum.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/438_SC_levelcon/SC_levelcon.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/439/439.c", cflags=cflags_dll_noopt_nocse_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/440_SC_totempol/SC_totempol.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/441_SC_Cloudrun/SC_Cloudrun.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_801dd170.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/442_SC_totempuz/SC_totempuz.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/443_SC_totembon/SC_totembon.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/444_SC_totemstr/SC_totemstr.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/445/445.c", cflags=cflags_dll_noopt),
            Object(Matching, "main/dll/draghead.c"),
            Object(Matching, "main/dll/lavaflow.c"),
            Object(MatchingFor("GSAE01"), "dlls/objects/323_FEseqobject/FEseqobject.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/322_FElevContro/FElevContro.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/324/324.c", cflags=cflags_dll_noopt),
            Object(Matching, "main/dll/dbpointmum.c"),
            Object(Matching, "main/dll/dbwaterflow.c"),
            Object(Matching, "main/dll/dblightgo.c"),
            Object(Matching, "main/dll/dbbullet.c"),
            Object(NonMatching, "dlls/objects/488_SB_Galleon/SB_Galleon.c", cflags=cflags_dll_noopt_noloopinv, mw_version="GC/1.3"),
            Object(Matching, "main/dll/dbdustgeezer.c"),
            Object(MatchingFor("GSAE01"), "dlls/objects/489_SB_Propelle/SB_Propelle.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/490_SB_ShipHead/SB_ShipHead.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/491_SB_ShipMast/SB_ShipMast.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/492_SB_ShipGun/SB_ShipGun.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/494_SB_CannonBa/SB_CannonBa.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/493_SB_FireBall/SB_FireBall.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/495_SB_CloudBal/SB_CloudBal.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/496_SB_KyteCage/SB_KyteCage.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/498_SB_CageKyte/SB_CageKyte.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/497_SB_SeqDoor/SB_SeqDoor.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/499_SB_MiniFire/SB_MiniFire.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/501/501.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/500/500.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/502/502.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/503_SB_ShipGunB/SB_ShipGunB.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/SP/dll_0285_spshop.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_801e66dc.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/trex_lazerwall.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/laserturretgroup.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0284_shopitem.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/SP/dll_0287_spscarab.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/SP/dll_0288_spdrape.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/SP/dll_0289_spitembeam.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_801e991c.c", cflags=cflags_dll_noopt_nocse),
            Object(NonMatching, "main/dll/drcloudcage.c", cflags=cflags_dll_noopt),
            Object(
                NonMatching, "main/dll/drcloudcage_engine.c", cflags=cflags_dll_noopt_nolifetimes,
                section_alignments={".sbss": 4},
            ),
            Object(NonMatching, "main/dll/drcloudcage_pitch.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/DR/drshackle.c", cflags=cflags_dll_noopt),
            Object(
                NonMatching, "main/dll/drhightop.c", cflags=cflags_dll_noopt,
                section_alignments={".data": 4},
            ),
            Object(NonMatching, "main/dll/drpickup.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/bwalphaanim.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/601_SB_Cloudrun/SB_Cloudrun.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/505_WM_ObjCreat/WM_ObjCreat.c", cflags=cflags_dll_noopt),
            Object(Matching, "main/dll/wcdial.c"),
            Object(MatchingFor("GSAE01"), "dlls/objects/504_WM_Galleon/WM_Galleon.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/506_WM_seqobjec/WM_seqobjec.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/507/507.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/508/508.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/510/510.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/511/511.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/509_WM_LaserTar/WM_LaserTar.c", cflags=cflags_dll_noopt_nostrength),
            Object(MatchingFor("GSAE01"), "dlls/objects/512/512.c", cflags=cflags_dll_noopt_nostrength_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/513_WM_colrise/WM_colrise.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/514/514.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/515/515.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/516_WM_Torch/WM_Torch.c", cflags=cflags_dll_noopt_nostrength),
            Object(MatchingFor("GSAE01"), "dlls/objects/517_WM_Vein/WM_Vein.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/518_LightSource/LightSource.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/519_WM_Worm/WM_Worm.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "dlls/objects/520_WM_Wallpowe/WM_Wallpowe.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/521_WM_LevelCon/WM_LevelCon.c", cflags=cflags_dll_noopt, mw_version="GC/1.3"),
            Object(MatchingFor("GSAE01"), "dlls/objects/522_WM_GeneralS/WM_GeneralS.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/604/604.c", cflags=cflags_dll_noopt_noloopinv_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/605_CRCloudRace/CRCloudRace.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/606/606.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/607_CRFuelTank/CRFuelTank.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/608/608.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "dlls/objects/609_DR_LaserCan/DR_LaserCan.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "dlls/objects/610/610.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "dlls/objects/611_GM_MazeWell/GM_MazeWell.c", cflags=cflags_dll_noopt),
            Object(Matching, "main/dll/dll_0266_kytesmum.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/DR/dll_0265_drcreator.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0269_explodeplan.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "main/dll/DR/dll_0268_drcagecontrol.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/DR/dll_026B_drchimmey.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/DR/dll_026C_drcagewith.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/DR/dll_026E_drshackle.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_0271_drakorhoverpad.c", cflags=cflags_dll_noopt_noautoinline),
            Object(NonMatching, "main/dll/dll_0271_drakorhoverpad_updatemain.c", cflags=cflags_dll_noopt_nocse_nodead_nofpcontract_noautoinline),
            Object(NonMatching, "main/dll/dll_0272_hightop.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/DR/dll_026F_drgenerator.c", cflags=cflags_dll_noopt),
            Object(Matching, "main/dll/dll_0273_firepipe.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/DR/dll_0279_drenergydisc.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/DR/dll_027C_drlightbea.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/DR/dll_027E_drmusiccont.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/DR/dll_0280_drcloudper.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/DR/dll_0281_drearthcal.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0282_barrelgener.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/obj_movelib.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/obj_movelib_steer.c", cflags=cflags_dll_noopt_level2),
            Object(NonMatching, "main/dll/obj_movelib_followcurve.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/obj_movelib_smoothturn.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "main/dll/DR/dll_0283_drbarrelgr.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/WC/dll_028A_wcearthwalker.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_028B.c", cflags=cflags_dll_noopt_nocse_noprop),
            Object(MatchingFor("GSAE01"), "main/dll/WC/dll_028C_wcbouncycra.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/WC/dll_0290_wcpushblock.c", cflags=[*cflags_base, "-opt", "nopeephole,noschedule,nocse,nodeadstore"]),
            Object(Matching, "main/dll/WC/dll_028D_wclevelcont.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/WC/dll_028E_wcbeacon.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "dlls/objects/655_WCPressureS/WCPressureS.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/WC/dll_0291_wctile.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "main/dll/WC/dll_0292_wctrexstatu.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0293_suntemple.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0294_wctemple.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_0299.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/WC/dll_0295_wcapertures.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/WC/dll_0296_wctempledia.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "main/dll/WC/dll_0297_wctemplebri.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/WC/dll_0298_wcfloortile.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/ARW/dll_029A_arwarwing.c", cflags=cflags_dll_noopt_noprop_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/dll_029B_arwingandrossstuff.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/ARW/dll_029C_arwarwingbo.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/ARW/dll_029D_arwarwinggu.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_029E_Dummy29E.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/ARW/dll_029F_arwbombcoll.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02A0_ring.c", cflags=cflags_dll_noopt_noinline),
            Object(MatchingFor("GSAE01"), "main/dll/ARW/dll_02A1_arwlevelcon.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/ARW/dll_02A2_arwspeedstr.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02A3.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02A4.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/ARW/dll_02A5_arwgenerato.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/ARW/dll_02A6_arwsquadron.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/ARW/dll_02A7_arwproximit.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "main/dll/ARW/dll_02A8_arwblocker.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/LGT/dll_02A9_lgtpointlight.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/LGT/dll_02AA_lgtdirectionallight.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/LGT/dll_02AB_lgtprojectedlight.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "main/dll/LGT/dll_02AC_lgtcontrollight.c", cflags=cflags_dll_noopt_level1),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02AD_softbody.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02AE_waterflowwe.c", cflags=cflags_dll_noopt, extra_cflags=["-opt", "nodeadstore"]),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02AF_tree.c", cflags=cflags_dll_noopt_nocse_noinline),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02B0_brokenpipe.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_02B1_cmbsrc.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02B2_dustmotesou.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/dll/dll_02B3_vortex.c", cflags=cflags_dll_noopt_noprop),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02B4_cntcounter.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02B5_timer.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02B6_cnthitobjec.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02B7_mcupgrade.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02B8_mcupgradema.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02B9_mcstaffeffe.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02BA_mclightning.c", cflags=cflags_dll_noopt),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02BB_gflevelcon.c", cflags=cflags_dll_noopt_noloopinv_noautoinline),
            Object(NonMatching, "main/dll/dll_02BC_andross.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02BD_androsshand.c", cflags=cflags_dll_noopt_noautoinline),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02BE_androssbrain.c", cflags=cflags_dll_noopt_nocse),
            Object(MatchingFor("GSAE01"), "main/dll/dll_02BF_androssligh.c", cflags=cflags_dll_noopt),
            Object(NonMatching, "main/audio/synth_queue.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/synth_seq_events.c", mw_version="GC/1.2.5n"),
            Object(NonMatching, "main/audio/synth_seq_dispatch.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on", "-fp_contract", "off"]),
            Object(NonMatching, "main/audio/synth_channel_scale.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on", "-fp_contract", "off"]),
            Object(NonMatching, "main/audio/synth.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on", "-fp_contract", "off"]),
            Object(MatchingFor("GSAE01"), "main/audio/synth_job_init.c", mw_version="GC/1.2.5n"),
            Object(NonMatching, "main/audio/synth_jobs.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/data_tables.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/mcmd_wait.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/mcmd_loop.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/mcmd_setup.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(NonMatching, "main/audio/mcmd_exec.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/vid_get.c"),
            Object(MatchingFor("GSAE01"), "main/audio/voice_id.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on", "-O2"]),
            Object(Matching, "main/audio/snd_groups.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on", "-inline", "noauto"]),
            Object(MatchingFor("GSAE01"), "main/audio/sal_studio.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/hw_dspctrl.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/sal_volume.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on", "-fp_contract", "off", "-inline", "all"]),
            Object(MatchingFor("GSAE01"), "main/audio/snd3dgroup.c", mw_version="GC/1.2.5n", extra_cflags=["-fp_contract", "off", "-Cpp_exceptions", "on", "-inline", "noauto"]),
            Object(MatchingFor("GSAE01"), "main/audio/snd_core.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on", "-fp_contract", "off"]),
            Object(MatchingFor("GSAE01"), "main/audio/hw_break.c"),
            Object(MatchingFor("GSAE01"), "main/audio/hw_sample.c"),
            Object(MatchingFor("GSAE01"), "main/audio/hw_keyoff.c"),
            Object(MatchingFor("GSAE01"), "main/audio/hw_volume.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/hw_stream.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/aram_queue.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/aram_init.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/aram_data.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/sal_ai.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/sal_dsp.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/sal_dsp_irqinit.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on", "-opt", "noschedule"]),
            Object(MatchingFor("GSAE01"), "main/audio/sal_dsp_irq.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "main/audio/snd_reverb.c", mw_version="GC/1.2.5n", extra_cflags=["-Cpp_exceptions", "on"]),
            Object(MatchingFor("GSAE01"), "dolphin/axfx/reverb_std_create.c", mw_version="GC/1.2.5n"),
            Object(
                MatchingFor("GSAE01"),
                "dolphin/MSL_C/PPCEABI/bare/H/math_float_helpers.c",
                mw_version="GC/1.2.5n",
                extra_cflags=["-inline", "off", *msl_math_extra],
                progress_category="sdk",
            ),
            Object(
                MatchingFor("GSAE01"),
                "main/acosf.c",
                mw_version="GC/1.2.5n",
                cflags=msl_math_o0_cflags,
                extra_cflags=["-O0", "-opt", "functions", "-inline", "auto", *msl_math_extra],
                progress_category="game",
            ),
            Object(
                NonMatching,
                "main/atan2f.c",
                mw_version="GC/1.2.5n",
                cflags=msl_math_o0_cflags,
                extra_cflags=["-O0", "-opt", "peephole,functions", "-inline", "auto", *msl_math_extra],
                progress_category="game",
            ),
            Object(
                MatchingFor("GSAE01"),
                "dolphin/MSL_C/PPCEABI/bare/H/e_sqrt.c",
                mw_version="GC/1.2.5n",
                cflags=msl_math_o0_cflags,
                extra_cflags=["-O0", "-opt", "peephole", "-inline", "auto", "-use_lmw_stmw", "on", *msl_math_extra],
                progress_category="sdk",
            ),
            Object(
                MatchingFor("GSAE01"),
                "dolphin/MSL_C/PPCEABI/bare/H/e_atan2.c",
                mw_version="GC/1.2.5n",
                extra_cflags=msl_math_extra,
                progress_category="sdk",
            ),
            Object(
                MatchingFor("GSAE01"),
                "dolphin/MSL_C/PPCEABI/bare/H/s_tan.c",
                mw_version="GC/1.2.5n",
                cflags=msl_math_o0_cflags,
                extra_cflags=["-O0", "-opt", "functions", "-inline", "auto", *msl_math_extra],
                progress_category="sdk",
            ),
            Object(
                MatchingFor("GSAE01"),
                "dolphin/MSL_C/PPCEABI/bare/H/trig_float_helpers.c",
                mw_version="GC/1.2.5n",
                cflags=msl_math_o0_cflags,
                extra_cflags=["-O0", "-opt", "functions,peephole", "-inline", "auto", *msl_math_extra],
                progress_category="sdk",
            ),
            Object(
                MatchingFor("GSAE01"),
                "dolphin/MSL_C/PPCEABI/bare/H/k_tan.c",
                mw_version="GC/1.2.5n",
                cflags=msl_math_o0_cflags,
                extra_cflags=["-O0", "-opt", "functions", "-inline", "auto", *msl_math_extra],
                progress_category="sdk",
            ),
            Object(
                NonMatching,
                "main/trig.c",
                mw_version="GC/1.2.5n",
                cflags=msl_math_o0_cflags,
                extra_cflags=["-O0", "-opt", "functions", "-inline", "auto", *msl_math_extra],
                progress_category="game",
            ),
            Object(
                MatchingFor("GSAE01"),
                "dolphin/MSL_C/PPCEABI/bare/H/math_80293da4.c",
                mw_version="GC/1.2.5n",
                cflags=msl_math_o0_cflags,
                extra_cflags=["-O0", "-opt", "functions,peephole", "-inline", "auto", *msl_math_extra],
                progress_category="sdk",
            ),
            Object(
                MatchingFor("GSAE01"),
                "dolphin/MSL_C/PPCEABI/bare/H/s_floor.c",
                mw_version="GC/1.2.5n",
                cflags=msl_math_o0_cflags,
                extra_cflags=["-O0", "-opt", "functions,peephole", "-inline", "auto", *msl_math_extra],
                progress_category="sdk",
            ),
            Object(
                MatchingFor("GSAE01"),
                "dolphin/MSL_C/PPCEABI/bare/H/s_sin.c",
                mw_version="GC/1.2.5n",
                cflags=msl_math_o0_cflags,
                extra_cflags=["-O0", "-opt", "functions", "-inline", "auto", *msl_math_extra],
                progress_category="sdk",
            ),
            Object(
                NonMatching,
                "dolphin/MSL_C/PPCEABI/bare/H/math_8029454c.c",
                mw_version="GC/1.2.5n",
                cflags=msl_math_o0_cflags,
                extra_cflags=["-O0", "-opt", "functions,peephole", "-inline", "auto", *msl_math_extra],
                progress_category="sdk",
            ),
        ],
    },
]


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    if not config.non_matching:
        return objects
    if module_id == 0:  # DOL
        return objects + ["dummy.c"]
    return objects


# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("game", "Game Code"),
    ProgressCategory("sdk", "SDK Code"),
]
config.progress_each_module = args.verbose
# Optional extra arguments to `objdiff-cli report generate`
config.progress_report_args = [
    # Marks relocations as mismatching if the target value is different
    # Default is "functionRelocDiffs=none", which is most lenient
    # "--config functionRelocDiffs=data_value",
]

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress information
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
