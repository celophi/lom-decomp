"""Read splat symbol files (name = 0xADDRESS;)."""

from __future__ import annotations

import re
from pathlib import Path

_LINE = re.compile(r"^\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+)\s*;")
_SIZE = re.compile(r"//.*\bsize:(0x[0-9A-Fa-f]+|\d+)")


def load(path: Path) -> dict[str, int]:
    """Map each symbol name to its address; commented-out lines are skipped."""
    symbols = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        match = _LINE.match(line)
        if match:
            symbols[match.group(1)] = int(match.group(2), 16)
    return symbols


def load_sizes(path: Path) -> dict[str, int]:
    """Map each symbol that declares a `// size:` to that size in bytes."""
    sizes = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        match, size = _LINE.match(line), _SIZE.search(line)
        if match and size:
            sizes[match.group(1)] = int(size.group(1), 0)
    return sizes
