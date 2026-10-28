#!/usr/bin/env python3
"""Write ZUKAN's entry tables as the initializer lists zukan_category.c includes.

zukan_build_category_entries keeps its four tables in local arrays that take
their size from the initializer, like ``s32 category_ranges[] = { ... };``.
The compiler stores those values in the file's own rodata and copies them onto
the stack on every call, so they can't be a separate databin the C refers to.
The values are game data, though, so the repository doesn't carry them.
``make splat`` runs this tool to read them from the decompressed overlay and
write one list per array to ``assets/<version>/zukan_category/``, which the
build adds to the include path.

Example:

    python3 -m tools.data.overlays.zukan_initializers --version us
"""

from __future__ import annotations

import argparse
from pathlib import Path
import struct
import sys

from tools.data.overlays import zukan

REPO_ROOT = Path(__file__).resolve().parents[3]
VALUES_PER_LINE = 16
# Extractor table key, C array name and element format, in rodata order.
ARRAYS = (
    ("category_ranges", "category_ranges", "i"),
    ("group_ranges", "group_ranges", "i"),
    ("entry_values", "resource_values", "h"),
    ("display_order", "display_order", "H"),
)


def initializer(values: list[int]) -> str:
    """The values as comma-separated C initializer lines."""
    lines = [", ".join(str(value) for value in values[start : start + VALUES_PER_LINE]) + ","
             for start in range(0, len(values), VALUES_PER_LINE)]
    return "\n".join(lines) + "\n"


def write_initializers(config: Path, version: str, output: Path) -> None:
    blob = zukan.load_tables(config, version)
    lengths = zukan.table_lengths(blob)
    output.mkdir(parents=True, exist_ok=True)
    for key, name, element in ARRAYS:
        start = blob.offset(getattr(blob.symbols, key))
        values = struct.unpack_from(f"<{lengths[key]}{element}", blob.data, start)
        (output / f"{name}.inc").write_text(initializer(list(values)), encoding="ascii")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--version", choices=("us", "jp"), default="us")
    parser.add_argument("--config", type=Path, help="config folder (default config/<version>)")
    parser.add_argument("--output", type=Path, help="folder to write (default assets/<version>/zukan_category)")
    args = parser.parse_args()
    config = args.config or REPO_ROOT / "config" / args.version
    output = args.output or REPO_ROOT / "assets" / args.version / "zukan_category"
    try:
        write_initializers(config, args.version, output)
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
