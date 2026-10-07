"""Find an overlay's data files from its splat config.

Splat writes each ``databin`` and ``rodatabin`` subsegment to
``<assets>/<name>.<type>.bin``. Reading the config tells us where each of those
files sits in memory, so tools can find data by address instead of by file name.
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


def data_files(config: Path, assets: Path) -> list[DataFile]:
    """List the overlay's data files in address order.

    Each subsegment runs until the next one starts; the last runs to the
    segment's end marker, the offset-only entry after the segment.
    """
    document = yaml.safe_load(config.read_text(encoding="utf-8"))
    segment = document["segments"][0]
    end_marker = document["segments"][1][0]
    subsegments = [entry for entry in segment["subsegments"] if isinstance(entry[0], int)]
    boundaries = [entry[0] for entry in subsegments[1:]] + [end_marker]

    def address(rom_offset: int) -> int:
        return segment["vram"] + rom_offset - segment["start"]

    files = []
    for (rom_offset, kind, name, *_), next_offset in zip(subsegments, boundaries):
        if kind in DATA_TYPES:
            files.append(
                DataFile(
                    name,
                    kind,
                    address(rom_offset),
                    address(next_offset),
                    assets / f"{name}.{kind}.bin",
                )
            )
    return files


def file_containing(files: list[DataFile], address: int, what: str) -> DataFile:
    """The data file that holds @p address; @p what names it in the error."""
    for data_file in files:
        if data_file.contains(address):
            return data_file
    raise ValueError(f"no data file in the splat config holds {what} (0x{address:08X})")
