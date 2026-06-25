#!/usr/bin/env python3
"""
find_small_funcs.py -- List the smallest undecompiled functions for an overlay
or the SLUS main executable, sorted by size ascending.

Usage:
    python tools/find_small_funcs.py field
    python tools/find_small_funcs.py slus
    python tools/find_small_funcs.py field --top 20

The overlay name should match the directory under asm/overlays/ (e.g. "field",
"menu", "addhero") or use "slus" for the main executable's nonmatchings under
asm/nonmatchings/.

Output columns:
    size (bytes)  function name  .s file path (relative to repo root)
"""

import argparse
import re
import sys
from pathlib import Path

# Pattern for the first line of every nonmatching .s file:
#   nonmatching func_XXXXXXXX, 0xSIZE
_HEADER_RE = re.compile(r"^\s*nonmatching\s+(\S+),\s+(0x[0-9A-Fa-f]+|\d+)")


def find_nonmatchings(asm_root: Path) -> list[tuple[int, str, Path]]:
    """Return (size_bytes, func_name, path) for every .s under asm_root."""
    results = []
    for s_file in sorted(asm_root.rglob("*.s")):
        try:
            first_line = s_file.read_text(encoding="utf-8", errors="replace").splitlines()[0]
        except IndexError:
            continue
        m = _HEADER_RE.match(first_line)
        if not m:
            continue
        func_name = m.group(1)
        size = int(m.group(2), 0)
        results.append((size, func_name, s_file))
    return results


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        "overlay",
        help='Overlay directory name (e.g. "field", "menu") or "slus" for the main executable.',
    )
    parser.add_argument(
        "--top",
        type=int,
        default=10,
        help="How many functions to show (default: 10).",
    )
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parent.parent

    if args.overlay.lower() == "slus":
        asm_root = repo_root / "asm" / "nonmatchings"
        label = "SLUS (main executable)"
    else:
        asm_root = repo_root / "asm" / "overlays" / args.overlay / "nonmatchings"
        label = args.overlay

    if not asm_root.exists():
        sys.exit(f"Error: nonmatchings directory not found: {asm_root}")

    funcs = find_nonmatchings(asm_root)
    if not funcs:
        print(f"No undecompiled functions found under {asm_root}")
        return

    funcs.sort(key=lambda t: t[0])

    top = funcs[: args.top]
    total = len(funcs)

    print(f"Smallest undecompiled functions in {label}  ({total} remaining)")
    print(f"{'Size':>8}  {'Function':<40}  File")
    print("-" * 90)
    for size, name, path in top:
        rel = path.relative_to(repo_root)
        print(f"{size:>8}  {name:<40}  {rel}")


if __name__ == "__main__":
    main()
