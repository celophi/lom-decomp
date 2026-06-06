#!/usr/bin/env python3
"""
Convert objdiff JSON output to a compact side-by-side text format.

Each non-100% function is rendered as:

  === func_name  91.22% ===
    0x4E4  addiu sp, sp, -0x58              |  0x4E4  addiu sp, sp, -0x58
  ! 0x500  <gap>                            |  0x500  addiu a3, a3, %lo(fn)   L366
  ! 0x550  lw t0, 0x5c(sp)                 |  0x550  lw t1, 0x5c(sp)         L366

Lines prefixed with '!' differ. Gaps (insert/delete) show '<gap>' on the
missing side. The right-side source line number is appended when available.

Usage:
    python3 tools/objdiff/format_diffs.py build/diffs/main/cdrom.json
    python3 tools/objdiff/format_diffs.py build/diffs/main/cdrom.json -o out.txt
    python3 tools/objdiff/format_diffs.py build/diffs/main/cdrom.json --all
"""

import argparse
import json
import sys
from pathlib import Path


def fmt_instr(entry: dict) -> tuple[str, str]:
    """Return (address_hex, formatted_text) for one instruction entry.

    An entry is a gap (no instruction on this side) when the 'instruction'
    key is absent -- diff_kind alone does not determine this.
    """
    instr = entry.get("instruction")
    if instr is None:
        return ("", "<gap>")
    addr = instr.get("address", "")
    addr_hex = f"0x{int(addr):X}" if addr else ""
    return (addr_hex, instr.get("formatted", "?"))


def is_mismatch(entry: dict) -> bool:
    """Any entry tagged with diff_kind represents a difference."""
    return "diff_kind" in entry


def format_unit(left_sym: dict, right_sym: dict, show_all: bool) -> list[str]:
    name = left_sym["name"]
    pct = left_sym.get("match_percent", 0.0) or 0.0
    lines = []
    lines.append(f"\n=== {name}  {pct:.2f}% ===")

    left_instrs = left_sym.get("instructions", [])
    right_instrs = right_sym.get("instructions", []) if right_sym else []

    # Pad shorter side with gap entries so zip covers all rows.
    gap = {"diff_kind": "DIFF_INSERT"}
    length = max(len(left_instrs), len(right_instrs))
    left_instrs  = left_instrs  + [gap] * (length - len(left_instrs))
    right_instrs = right_instrs + [gap] * (length - len(right_instrs))

    COL = 42  # width of each asm column
    for le, re in zip(left_instrs, right_instrs):
        mismatch = is_mismatch(le) or is_mismatch(re)
        if not show_all and not mismatch:
            continue

        mark = "!" if mismatch else " "
        la, lf = fmt_instr(le)
        ra, rf = fmt_instr(re)
        src_line = re.get("instruction", {}).get("line_number", "")
        src_tag = f"  L{src_line}" if src_line else ""

        left_col  = f"{la:6}  {lf}"
        right_col = f"{ra:6}  {rf}"
        lines.append(f"  {mark} {left_col:{COL}} | {right_col}{src_tag}")

    return lines


def format_file(path: Path, show_all: bool) -> list[str]:
    with open(path, "rb") as f:
        data = json.load(f)

    left_by_name  = {s["name"]: s for s in data.get("left",  {}).get("symbols", [])}
    right_by_name = {s["name"]: s for s in data.get("right", {}).get("symbols", [])}

    # Overall section match for the header line.
    text_section = next(
        (s for s in data.get("left", {}).get("sections", []) if s["name"] == ".text"),
        {},
    )
    section_pct = text_section.get("match_percent", 0.0) or 0.0

    out = [f"# {path}  (.text {section_pct:.2f}%)"]

    for name, left_sym in left_by_name.items():
        sym_pct = left_sym.get("match_percent", 0.0) or 0.0
        if not show_all and sym_pct >= 100.0:
            continue
        right_sym = right_by_name.get(name)
        out.extend(format_unit(left_sym, right_sym, show_all))

    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("input", help="objdiff JSON file")
    parser.add_argument("-o", "--output", help="Output file (default: stdout)")
    parser.add_argument("--all", action="store_true",
                        help="Include 100% matched functions and matching lines")
    args = parser.parse_args()

    lines = format_file(Path(args.input), show_all=args.all)
    text = "\n".join(lines) + "\n"

    if args.output:
        Path(args.output).write_text(text, encoding="utf-8")
    else:
        sys.stdout.write(text)


if __name__ == "__main__":
    main()
