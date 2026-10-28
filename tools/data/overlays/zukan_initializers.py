#!/usr/bin/env python3
"""Write ZUKAN's entry tables out as C macros for zukan_category.c.

zukan_build_category_entries copies four tables onto the stack every time it
runs: the category boundaries, the World History groups, each entry's page and
the order the lists are shown in. The original code had them as local arrays
with initializers, and GCC only produces that exact copy code when it sees the
values right there in the initializer. Pointing the C at a databin with an
extern gives you different instructions, so that's out.

The values are game data though, and we don't keep game data in the repository.
So make splat runs this script instead. It reads the tables from your own
decompressed ZUKAN.BIN and writes assets/<version>/zukan_category/tables.h,
one macro per table:

    #define ZUKAN_CATEGORY_RANGES \\
        <values, 16 to a line>

and the C uses them like this:

    s32 category_ranges[] = {ZUKAN_CATEGORY_RANGES};

Usage:

    python3 -m tools.data.overlays.zukan_initializers --version us
"""

from __future__ import annotations

import argparse
from pathlib import Path
import struct
import sys

from tools.data.overlays import zukan

REPO_ROOT = Path(__file__).resolve().parents[3]
HEADER = "zukan_category/tables.h"
VALUES_PER_LINE = 16

# The extractor's name for each table, how its values are stored, and the macro
# the C uses for it. They have to stay in this order, since it's the order the
# tables sit in the overlay.
TABLES = (
    ("category_ranges", "i", "ZUKAN_CATEGORY_RANGES"),
    ("group_ranges", "i", "ZUKAN_GROUP_RANGES"),
    ("entry_values", "h", "ZUKAN_RESOURCE_VALUES"),
    ("display_order", "H", "ZUKAN_DISPLAY_ORDER"),
)


def define(macro: str, values: tuple[int, ...]) -> str:
    lines = []
    for start in range(0, len(values), VALUES_PER_LINE):
        lines.append(", ".join(str(value) for value in values[start : start + VALUES_PER_LINE]))
    return f"#define {macro} \\\n    " + ", \\\n    ".join(lines) + "\n"


def write_header(version: str) -> None:
    blob = zukan.load_tables(REPO_ROOT / "config" / version, version)
    lengths = zukan.table_lengths(blob)

    defines = []
    for name, element, macro in TABLES:
        start = blob.offset(getattr(blob.symbols, name))
        values = struct.unpack_from(f"<{lengths[name]}{element}", blob.data, start)
        defines.append(define(macro, values))

    path = REPO_ROOT / "assets" / version / HEADER
    path.parent.mkdir(parents=True, exist_ok=True)
    text = "#ifndef ZUKAN_CATEGORY_TABLES_H\n#define ZUKAN_CATEGORY_TABLES_H\n\n"
    text += "\n".join(defines)
    text += "\n#endif\n"
    path.write_text(text, encoding="ascii")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--version", choices=("us", "jp"), default="us")
    args = parser.parse_args()
    try:
        write_header(args.version)
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
