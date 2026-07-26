#!/usr/bin/env python3
"""Regenerate empty DLL source folders using retail-only names."""

from __future__ import annotations

import argparse
import re
import struct
from collections import defaultdict
from pathlib import Path


REPO = Path(__file__).resolve().parent.parent
DLLS_ROOT = REPO / "src/dlls"
DLLS_TXT = DLLS_ROOT / "dlls.txt"
DEFAULT_DOL = REPO / "orig/GSAE01/sys/main.dol"
DEFAULT_FILES_ROOT = REPO.parent / "sfa-extracted"

BANKS = (
    ("engine", 0, 88),
    ("modgfx", 89, 170),
    ("projgfx", 171, 194),
    ("objects", 195, 704),
)

MAPPING_RE = re.compile(r"^(\d+)\s*=\s*([^/]+)/(\S+)$")
LEAF_RE = re.compile(r"^(\d+)(?:_(.+))?$")
SAFE_NAME_RE = re.compile(r"^[A-Za-z0-9_]+$")
PRINTABLE_RE = re.compile(rb"[ -~]{4,}")
SOURCE_BASENAME_RE = re.compile(
    r"(?<![A-Za-z0-9_])([A-Za-z_][A-Za-z0-9_]*)\.c(?![A-Za-z0-9_])"
)


def bank_for_slot(slot: int) -> str:
    for bank, first, last in BANKS:
        if first <= slot <= last:
            return bank
    raise ValueError(f"slot {slot} is outside the scaffold range")


def load_existing_mappings() -> dict[int, tuple[str, str]]:
    mappings: dict[int, tuple[str, str]] = {}
    for line in DLLS_TXT.read_text(encoding="utf-8").splitlines():
        match = MAPPING_RE.match(line)
        if match is None:
            continue
        slot = int(match.group(1))
        mappings[slot] = (match.group(2), match.group(3))
    expected = set(range(705))
    if set(mappings) != expected:
        missing = sorted(expected - set(mappings))
        extra = sorted(set(mappings) - expected)
        raise SystemExit(f"dlls.txt slot mismatch: missing={missing}, extra={extra}")
    return mappings


def load_retail_source_basenames(dol_path: Path) -> set[str]:
    data = dol_path.read_bytes()
    names: set[str] = set()
    for match in PRINTABLE_RE.finditer(data):
        text = match.group().decode("ascii", errors="ignore")
        names.update(source.group(1) for source in SOURCE_BASENAME_RE.finditer(text))
    return names


def load_retail_object_names(files_root: Path) -> dict[int, list[str]]:
    objects = (files_root / "OBJECTS.bin").read_bytes()
    table = (files_root / "OBJECTS.tab").read_bytes()
    offsets: list[int] = []
    for offset in range(0, len(table), 4):
        value = struct.unpack_from(">I", table, offset)[0]
        if value == 0xFFFFFFFF:
            break
        offsets.append(value)
    if len(offsets) < 2:
        raise SystemExit("OBJECTS.tab does not contain a valid offset table")

    names: dict[int, list[str]] = defaultdict(list)
    for offset in offsets[:-1]:
        dll_id = struct.unpack_from(">H", objects, offset + 0x50)[0]
        if dll_id == 0xFFFF or dll_id > 704:
            continue
        name = (
            objects[offset + 0x91 : offset + 0x9C]
            .split(b"\0", 1)[0]
            .decode("ascii", errors="strict")
        )
        if name and name not in names[dll_id]:
            names[dll_id].append(name)
    return dict(names)


def inspect_leaf(path: Path) -> tuple[int, bool]:
    match = LEAF_RE.fullmatch(path.name)
    if match is None:
        raise SystemExit(f"unexpected DLL scaffold directory name: {path}")
    slot = int(match.group(1))
    if bank_for_slot(slot) != path.parent.name:
        raise SystemExit(f"DLL slot {slot} is in the wrong bank: {path}")

    entries = list(path.iterdir())
    payload = [entry for entry in entries if entry.name != ".gitkeep"]
    if payload:
        return slot, True
    if len(entries) > 1 or (entries and not entries[0].is_file()):
        raise SystemExit(f"empty scaffold directory has unexpected contents: {path}")
    return slot, False


def inspect_scaffold() -> tuple[dict[int, Path], list[Path]]:
    populated: dict[int, Path] = {}
    empty: list[Path] = []
    for bank, _, _ in BANKS:
        bank_path = DLLS_ROOT / bank
        for path in sorted(bank_path.iterdir()):
            if not path.is_dir():
                raise SystemExit(f"unexpected non-directory in scaffold bank: {path}")
            slot, has_payload = inspect_leaf(path)
            if has_payload:
                if slot in populated:
                    raise SystemExit(
                        f"multiple populated folders claim DLL slot {slot}: "
                        f"{populated[slot]} and {path}"
                    )
                populated[slot] = path
            else:
                empty.append(path)
    return populated, empty


def existing_candidate(slot: int, mappings: dict[int, tuple[str, str]]) -> str | None:
    bank, leaf = mappings[slot]
    if bank != bank_for_slot(slot):
        raise SystemExit(f"dlls.txt maps slot {slot} to the wrong bank: {bank}")
    match = LEAF_RE.fullmatch(leaf)
    if match is None or int(match.group(1)) != slot:
        raise SystemExit(f"dlls.txt has an invalid leaf for slot {slot}: {leaf}")
    return match.group(2)


def retail_name_for_slot(
    slot: int,
    candidate: str | None,
    dol_source_names: set[str],
    object_names: dict[int, list[str]],
) -> tuple[str | None, str]:
    names = object_names.get(slot, [])
    if candidate in dol_source_names:
        return candidate, "DOL source basename"
    if candidate in names:
        return candidate, "OBJECTS.bin exact name"
    if len(names) == 1 and SAFE_NAME_RE.fullmatch(names[0]):
        return names[0], "OBJECTS.bin sole exact name"
    return None, "number-only"


def render_mappings(leaves: dict[int, str]) -> str:
    lines: list[str] = []
    for bank, first, last in BANKS:
        lines.append(f"# {bank}")
        for slot in range(first, last + 1):
            lines.append(f"{slot} = {bank}/{leaves[slot]}")
        if last != BANKS[-1][2]:
            lines.append("")
    return "\n".join(lines) + "\n"


def remove_empty_leaf(path: Path) -> None:
    keep = path / ".gitkeep"
    if keep.exists():
        keep.unlink()
    path.rmdir()


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Regenerate empty DLL folders using exact retail evidence only."
    )
    parser.add_argument("--dol", type=Path, default=DEFAULT_DOL)
    parser.add_argument("--files-root", type=Path, default=DEFAULT_FILES_ROOT)
    parser.add_argument(
        "--check",
        action="store_true",
        help="Report whether regeneration is needed without changing files.",
    )
    args = parser.parse_args()

    mappings = load_existing_mappings()
    populated, empty = inspect_scaffold()
    dol_source_names = load_retail_source_basenames(args.dol)
    object_names = load_retail_object_names(args.files_root)

    leaves: dict[int, str] = {}
    reasons: dict[str, int] = defaultdict(int)
    for slot in range(705):
        if slot in populated:
            leaves[slot] = populated[slot].name
            reasons["populated"] += 1
            continue
        candidate = existing_candidate(slot, mappings)
        name, reason = retail_name_for_slot(
            slot, candidate, dol_source_names, object_names
        )
        leaves[slot] = f"{slot}_{name}" if name is not None else str(slot)
        reasons[reason] += 1

    desired_text = render_mappings(leaves)
    current_dirs = {
        path.relative_to(DLLS_ROOT).as_posix()
        for bank, _, _ in BANKS
        for path in (DLLS_ROOT / bank).iterdir()
        if path.is_dir()
    }
    desired_dirs = {
        f"{bank_for_slot(slot)}/{leaf}" for slot, leaf in leaves.items()
    }
    needs_update = current_dirs != desired_dirs or DLLS_TXT.read_text(
        encoding="utf-8"
    ) != desired_text

    if args.check:
        if needs_update:
            raise SystemExit("DLL scaffold differs from the retail-only mapping")
        print("DLL scaffold matches the retail-only mapping")
        return

    for path in empty:
        remove_empty_leaf(path)
    for slot, leaf in leaves.items():
        if slot in populated:
            continue
        path = DLLS_ROOT / bank_for_slot(slot) / leaf
        path.mkdir()
        (path / ".gitkeep").write_bytes(b"")
    DLLS_TXT.write_text(desired_text, encoding="utf-8")

    print(f"preserved populated folders: {reasons['populated']}")
    print(f"named from DOL source basenames: {reasons['DOL source basename']}")
    print(f"named from exact OBJECTS.bin candidates: {reasons['OBJECTS.bin exact name']}")
    print(f"named from sole exact OBJECTS.bin names: {reasons['OBJECTS.bin sole exact name']}")
    print(f"left number-only: {reasons['number-only']}")


if __name__ == "__main__":
    main()
