#!/usr/bin/env python3
"""
Extract a single function and its jump tables from splat-generated .s files.

Usage (run from /staging):
    python3 tools/extract_func.py asm/cdrom.s cdrom_complete_command
    python3 tools/extract_func.py asm/cdrom.s cdrom_complete_command -o /tmp/target.s

The output is a self-contained .s file with:
  - Standard MIPS header
  - .section .text  +  the target function
  - .section .rodata  +  any jump tables referenced by the function (if any)
"""

import argparse
import re
import sys
from pathlib import Path

# Standard header for all splat-generated .s files in this project
HEADER = """.include "macro.inc"

.set noat
.set noreorder

"""

# Rodata files to search for jump tables (main game, in order)
MAIN_RODATA_FILES = [
    "asm/data/rodata1.rodata.s",
    "asm/data/rodata2.rodata.s",
]

JTBL_RE = re.compile(r'\bjtbl_[0-9A-Fa-f]+\b')


def extract_text_block(lines: list[str], func_name: str) -> list[str]:
    """Extract lines from glabel <func_name> to endlabel <func_name> inclusive."""
    start_marker = f"glabel {func_name}"
    end_marker = f"endlabel {func_name}"

    capturing = False
    result = []

    for line in lines:
        stripped = line.strip()
        if not capturing:
            if stripped == start_marker:
                capturing = True
                result.append(line)
        else:
            result.append(line)
            if stripped == end_marker:
                return result

    if not result:
        return []  # not found
    # Found glabel but no endlabel
    print(f"WARNING: found glabel {func_name} but never found endlabel {func_name}", file=sys.stderr)
    return result


def find_jtbl_names(func_lines: list[str]) -> list[str]:
    """Return unique jump table names referenced in the function, in order of appearance."""
    seen = set()
    ordered = []
    for line in func_lines:
        for match in JTBL_RE.finditer(line):
            name = match.group(0)
            if name not in seen:
                seen.add(name)
                ordered.append(name)
    return ordered


def extract_rodata_block(lines: list[str], jtbl_name: str) -> list[str]:
    """Extract lines from dlabel <jtbl_name> to enddlabel <jtbl_name> inclusive."""
    start_marker = f"dlabel {jtbl_name}"
    end_marker = f"enddlabel {jtbl_name}"

    capturing = False
    result = []

    for line in lines:
        stripped = line.strip()
        if not capturing:
            if stripped == start_marker:
                capturing = True
                result.append(line)
        else:
            result.append(line)
            if stripped == end_marker:
                return result

    return []  # not found


def find_rodata_files(asm_file: Path) -> list[Path]:
    """
    Auto-discover rodata files to search.

    Strategy:
    1. Look for a data/ directory alongside the asm file (overlay case):
       asm/overlays/menu/menu.s  ->  asm/overlays/menu/data/*.rodata.s
    2. Walk up to the asm/ root and check asm/data/ (main game case):
       asm/cdrom.s  ->  asm/data/rodata1.rodata.s, asm/data/rodata2.rodata.s
    3. Fall back to the hardcoded main rodata list.

    Returns paths in search priority order (most local first).
    """
    found: list[Path] = []

    # Check sibling data/ directory (overlay)
    sibling_data = asm_file.parent / "data"
    if sibling_data.is_dir():
        found.extend(sorted(sibling_data.glob("*.rodata.s")))

    # Walk up to find an asm/data/ directory (main game)
    parent = asm_file.parent
    while True:
        candidate = parent / "data"
        if candidate.is_dir() and candidate != sibling_data:
            found.extend(sorted(candidate.glob("*.rodata.s")))
        if parent.name == "asm" or parent == parent.parent:
            break
        parent = parent.parent

    # Fallback: hardcoded main rodata paths
    if not found:
        for path in MAIN_RODATA_FILES:
            p = Path(path)
            if p.exists():
                found.append(p)

    return found


def load_rodata_files(paths: list[Path]) -> list[tuple[str, list[str]]]:
    """Load rodata files. Returns list of (path_str, lines)."""
    result = []
    for p in paths:
        if not p.exists():
            print(f"WARNING: rodata file not found: {p}", file=sys.stderr)
            continue
        result.append((str(p), p.read_text(encoding="utf-8").splitlines(keepends=True)))
    return result


def main():
    parser = argparse.ArgumentParser(
        description="Extract a single function + its jump tables from splat .s output."
    )
    parser.add_argument("asm_file", help="Path to the .s file containing the function (e.g. asm/cdrom.s)")
    parser.add_argument("func_name", help="Name of the function to extract (e.g. cdrom_complete_command)")
    parser.add_argument("-o", "--out", default=None,
                        help="Output file path (default: print to stdout)")
    parser.add_argument("--rodata", nargs="*", default=None,
                        metavar="FILE",
                        help="Rodata files to search for jump tables (default: auto-discovered)")
    args = parser.parse_args()

    asm_path = Path(args.asm_file)
    if not asm_path.exists():
        print(f"ERROR: asm file not found: {args.asm_file}", file=sys.stderr)
        sys.exit(1)

    # --- Rodata file discovery ---
    if args.rodata is not None:
        rodata_file_list = load_rodata_files([Path(p) for p in args.rodata])
    else:
        discovered = find_rodata_files(asm_path)
        if discovered:
            print(f"Rodata search path: {', '.join(str(p) for p in discovered)}", file=sys.stderr)
        rodata_file_list = load_rodata_files(discovered)

    # --- Extract function text ---
    asm_lines = asm_path.read_text(encoding="utf-8").splitlines(keepends=True)
    func_lines = extract_text_block(asm_lines, args.func_name)

    if not func_lines:
        print(f"ERROR: function '{args.func_name}' not found in {args.asm_file}", file=sys.stderr)
        print("       (looking for 'glabel {func_name}' / 'endlabel {func_name}')", file=sys.stderr)
        sys.exit(1)

    print(f"Extracted {len(func_lines)} lines for {args.func_name}", file=sys.stderr)

    # --- Find jump tables ---
    jtbl_names = find_jtbl_names(func_lines)
    if jtbl_names:
        print(f"Found jump table references: {', '.join(jtbl_names)}", file=sys.stderr)
    else:
        print("No jump table references found.", file=sys.stderr)

    # --- Extract rodata blocks ---
    rodata_blocks: list[tuple[str, list[str]]] = []  # (jtbl_name, lines)

    if jtbl_names:
        for jtbl_name in jtbl_names:
            found = False
            for path, lines in rodata_file_list:
                block = extract_rodata_block(lines, jtbl_name)
                if block:
                    print(f"  {jtbl_name}: found in {path} ({len(block)} lines)", file=sys.stderr)
                    rodata_blocks.append((jtbl_name, block))
                    found = True
                    break
            if not found:
                print(f"  WARNING: {jtbl_name} not found in any rodata file", file=sys.stderr)

    # --- Assemble output ---
    out_parts = [HEADER]
    out_parts.append(".section .text, \"ax\"\n\n")
    out_parts.extend(func_lines)

    if rodata_blocks:
        out_parts.append("\n.section .rodata\n\n")
        for jtbl_name, block in rodata_blocks:
            out_parts.extend(block)
            out_parts.append("\n")

    output = "".join(out_parts)

    if args.out:
        Path(args.out).write_text(output, encoding="utf-8")
        print(f"Written to {args.out}", file=sys.stderr)
    else:
        sys.stdout.write(output)


if __name__ == "__main__":
    main()
