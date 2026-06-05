#!/usr/bin/env python3
"""
Decompile unmatched menu overlay functions using m2c.

Runs m2c on unk1.s together with the overlay's rodata (which holds
jumptables), so switch statements decompile correctly.

Usage:
    python3 tools/m2c_decompile.py                    # all 64 functions
    python3 tools/m2c_decompile.py -f func_8014519C   # one function

Context (optional, improves type output):
    m2c requires a *preprocessed* C file, not a raw header.  Generate one with:
        gcc -E -Iinclude include/menu.h -o /tmp/menu_ctx.c
    Then pass it:
        python3 tools/m2c_decompile.py --context /tmp/menu_ctx.c
"""

import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
M2C  = ROOT / "tools" / "m2c" / "m2c.py"

# Text section with all unmatched menu functions.
TEXT_FILE  = ROOT / "asm" / "overlays" / "menu" / "unk1.s"
# Rodata section -- contains jumptables referenced by TEXT_FILE.
RODATA_FILE = ROOT / "asm" / "overlays" / "menu" / "data" / "rodata.rodata.s"

# PSX target: MIPS O32 ABI, GCC 2.8.0, C language.
TARGET = "mips-gcc-c"

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Decompile menu unk1 functions via m2c"
    )
    parser.add_argument(
        "-f", "--function",
        metavar="NAME",
        help="Decompile only this function (e.g. func_8014519C). "
             "Omit to decompile all functions in unk1.s.",
    )
    parser.add_argument(
        "--context",
        metavar="FILE",
        default=None,
        help="Preprocessed C context file for type information. "
             "Must be the output of 'gcc -E', not a raw header. "
             "Optional -- omit to get untyped output.",
    )
    parser.add_argument(
        "--no-cache",
        action="store_true",
        help="Disable m2c's context cache (.m2c files).",
    )
    args = parser.parse_args()

    if not M2C.exists():
        print("error: tools/m2c/m2c.py not found -- run: git submodule update --init tools/m2c", file=sys.stderr)
        sys.exit(1)

    for path in (TEXT_FILE, RODATA_FILE):
        if not path.exists():
            print(f"error: {path} not found -- run splat to regenerate asm", file=sys.stderr)
            sys.exit(1)

    cmd = [
        sys.executable, str(M2C),
        "-t", TARGET,
    ]
    if args.context:
        cmd += ["--context", args.context]
    if args.function:
        cmd += ["-f", args.function]
    if args.no_cache:
        cmd += ["--no-cache"]
    # Pass text first so m2c processes functions in source order,
    # then rodata so jumptables are available during analysis.
    cmd += [str(TEXT_FILE), str(RODATA_FILE)]

    result = subprocess.run(cmd)
    sys.exit(result.returncode)


if __name__ == "__main__":
    main()
