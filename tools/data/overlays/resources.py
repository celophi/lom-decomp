"""Shared byte maps and output writers for overlay resource exports.

An overlay supplies the readers for its own data. Parts describe the ranges
those readers recognize; gaps are kept as padding or saved unchanged.
"""

from __future__ import annotations

from dataclasses import dataclass
from functools import singledispatch
from pathlib import Path
from typing import Generic, TypeVar

import yaml

SymbolTable = TypeVar("SymbolTable")


@dataclass(frozen=True)
class Blob(Generic[SymbolTable]):
    """The data blob and where it sits in memory."""

    data: bytes
    address: int  # memory address of data[0]
    file_name: str
    version: str
    symbols: SymbolTable

    def offset(self, address: int) -> int:
        """Blob offset of a memory address."""
        return address - self.address


@dataclass(frozen=True)
class Part:
    """One range of the blob; end is exclusive.

    content is what gets written for the part, or None when the byte map entry
    and its note are all there is. file is where the content goes.
    """

    name: str
    start: int
    end: int
    file: str | None = None
    content: object | None = None
    note: str | None = None


def cover_gaps(blob: Blob, parts: list[Part]) -> list[Part]:
    """Add the bytes between parts: zero padding, or unknown data saved as-is."""
    result = []
    cursor = 0
    for part in parts + [Part("end", len(blob.data), len(blob.data))]:
        if not 0 <= part.start <= part.end <= len(blob.data):
            raise ValueError(f"{part.name} is outside the data blob")
        if part.start < cursor:
            raise ValueError(f"{part.name} overlaps the part before it")
        if cursor < part.start:
            raw = blob.data[cursor : part.start]
            if any(raw):
                file = f"unknown/{blob.address + cursor:08X}.bin"
                result.append(Part("unknown", cursor, part.start, file, raw))
            else:
                result.append(Part("padding", cursor, part.start, note="zero bytes"))
        if part.name != "end":
            result.append(part)
        cursor = part.end
    return result


def hex_address(address: int) -> str:
    return f"0x{address:08X}"


def dump_yaml(path: Path, document: object, header: str = "") -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    body = yaml.safe_dump(document, sort_keys=False, allow_unicode=True, width=120)
    path.write_text(header + body, encoding="utf-8")


@singledispatch
def write_part(content: object, path: Path) -> None:
    """Write one part's content to @p path. Each content type registers its own writer."""
    raise TypeError(f"no writer for {type(content).__name__}")


@write_part.register
def _write_bytes(content: bytes, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(content)


def byte_map_entry(blob: Blob, part: Part) -> dict[str, str]:
    entry = {
        "name": part.name,
        "address": hex_address(blob.address + part.start),
        "offset": f"0x{part.start:X}",
        "size": f"0x{part.end - part.start:X}",
    }
    if part.file:
        entry["file"] = part.file
    if part.note:
        entry["note"] = part.note
    return entry
