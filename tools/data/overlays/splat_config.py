"""Find an overlay's data from its splat config.

Splat writes each ``databin`` and ``rodatabin`` subsegment to
``<assets>/<name>.<type>.bin``. Reading the config tells us where each of those
files sits in memory, so tools can find data by address instead of by file name.

Data that a C file compiles itself, like a function's local array initializers,
has no extracted file. Tools read those bytes from the decompressed overlay
that splat splits, through ``section`` and ``read_image``.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

import yaml

DATA_TYPES = ("databin", "rodatabin")


@dataclass(frozen=True)
class DataFile:
    """One extracted data file and the memory range it covers; end is exclusive."""

    name: str
    kind: str
    start: int
    end: int
    path: Path

    def contains(self, address: int) -> bool:
        return self.start <= address < self.end


def subsegments(config: Path) -> list[tuple[str, str, int, int]]:
    """List the overlay's subsegments as (name, kind, start, end) memory ranges.

    Each subsegment runs until the next one starts; the last runs to the
    segment's end marker, the offset-only entry after the segment.
    """
    document = yaml.safe_load(config.read_text(encoding="utf-8"))
    segment = document["segments"][0]
    end_marker = document["segments"][1][0]
    entries = [entry for entry in segment["subsegments"] if isinstance(entry[0], int)]
    boundaries = [entry[0] for entry in entries[1:]] + [end_marker]

    def address(rom_offset: int) -> int:
        return segment["vram"] + rom_offset - segment["start"]

    return [(name, kind, address(rom_offset), address(next_offset))
            for (rom_offset, kind, name, *_), next_offset in zip(entries, boundaries)]


def data_files(config: Path, assets: Path) -> list[DataFile]:
    """List the overlay's data files in address order."""
    return [DataFile(name, kind, start, end, assets / f"{name}.{kind}.bin")
            for name, kind, start, end in subsegments(config) if kind in DATA_TYPES]


def file_containing(files: list[DataFile], address: int, what: str) -> DataFile:
    """The data file that holds @p address; @p what names it in the error."""
    for data_file in files:
        if data_file.contains(address):
            return data_file
    raise ValueError(f"no data file in the splat config holds {what} (0x{address:08X})")


def section(config: Path, name: str, kind: str) -> tuple[int, int]:
    """The memory range of the subsegment @p kind of @p name, such as a C file's .rodata."""
    for entry_name, entry_kind, start, end in subsegments(config):
        if (entry_name, entry_kind) == (name, kind):
            return start, end
    raise ValueError(f"{config} has no {kind} subsegment for {name}")


def read_image(config: Path, start: int, end: int) -> bytes:
    """Read the memory range [start, end) from the decompressed overlay that splat splits."""
    document = yaml.safe_load(config.read_text(encoding="utf-8"))
    options = document["options"]
    segment = document["segments"][0]
    image = config.parent / options["base_path"] / options["target_path"]
    if not image.exists():
        raise ValueError(f"{image} is missing; run make splat first")
    raw = image.read_bytes()
    offset = start - segment["vram"] + segment["start"]
    if not 0 <= offset <= offset + (end - start) <= len(raw):
        raise ValueError(f"0x{start:08X}-0x{end:08X} is outside {image}")
    return raw[offset : offset + (end - start)]
