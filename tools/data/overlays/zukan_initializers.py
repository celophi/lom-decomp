#!/usr/bin/env python3
"""Write ZUKAN's entry tables as the initializer macros zukan_category.c uses.

zukan_build_category_entries keeps its four tables in local arrays that take
their size from the initializer, like
``s32 category_ranges[] = {ZUKAN_CATEGORY_RANGES};``.
The compiler stores those values in the file's own rodata and copies them onto
the stack on every call, so they can't be a separate databin the C refers to.
The values are game data, though, so the repository doesn't carry them.
``make splat`` runs this tool to read them from the decompressed overlay and
write one macro per array to ``assets/<version>/zukan_category/tables.h``. The
build adds ``assets/<version>`` to the include path.

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
HEADER = "zukan_category/tables.h"
GUARD = "ZUKAN_CATEGORY_TABLES_H"
# Extractor table key, C array name, element format and macro, in rodata order.
ARRAYS = (
    ("category_ranges", "category_ranges", "i", "ZUKAN_CATEGORY_RANGES"),
    ("group_ranges", "group_ranges", "i", "ZUKAN_GROUP_RANGES"),
    ("entry_values", "resource_values", "h", "ZUKAN_RESOURCE_VALUES"),
    ("display_order", "display_order", "H", "ZUKAN_DISPLAY_ORDER"),
)


def define(macro: str, values: list[int]) -> str:
    """A macro holding the values, VALUES_PER_LINE to a continued line."""
    lines = [", ".join(str(value) for value in values[start : start + VALUES_PER_LINE])
             for start in range(0, len(values), VALUES_PER_LINE)]
    return f"#define {macro} \\\n    " + ", \\\n    ".join(lines) + "\n"


def write_initializers(config: Path, version: str, assets: Path) -> None:
    blob = zukan.load_tables(config, version)
    lengths = zukan.table_lengths(blob)
    defines = []
    for key, _, element, macro in ARRAYS:
        start = blob.offset(getattr(blob.symbols, key))
        defines.append(define(macro, list(struct.unpack_from(f"<{lengths[key]}{element}", blob.data, start))))
    path = assets / HEADER
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(f"#ifndef {GUARD}\n#define {GUARD}\n\n" + "\n".join(defines) + f"\n#endif /* {GUARD} */\n",
                    encoding="ascii")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--version", choices=("us", "jp"), default="us")
    parser.add_argument("--config", type=Path, help="config folder (default config/<version>)")
    parser.add_argument("--assets", type=Path, help="assets folder to write into (default assets/<version>)")
    args = parser.parse_args()
    config = args.config or REPO_ROOT / "config" / args.version
    assets = args.assets or REPO_ROOT / "assets" / args.version
    try:
        write_initializers(config, args.version, assets)
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
