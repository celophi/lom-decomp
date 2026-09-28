"""Read overlay text tables: a u16 offset table followed by zero-terminated strings.

The first offset points just past the offset table, so it also gives the entry
count. See docs/en/technical/reference/text-tables.md.
"""

from __future__ import annotations

from dataclasses import dataclass

# Bytes followed by one more byte that belongs to them, even when that byte is
# zero. The US renderer only reads 0x19 (a two-byte character) and 0x1F (an
# extended dictionary entry) that way; the Japanese text uses 0x19-0x1F as
# lead bytes.
TWO_BYTE_CODES = {"us": frozenset((0x19, 0x1F)), "jp": frozenset(range(0x19, 0x20))}


@dataclass(frozen=True)
class TextEntry:
    index: int
    offset: int
    data: bytes
    two_byte_codes: frozenset[int]

    @property
    def text(self) -> str:
        return decode(self.data, self.two_byte_codes)


@dataclass(frozen=True)
class TextTable:
    entries: tuple[TextEntry, ...]
    size: int


def string_end(data: bytes, position: int, two_byte_codes: frozenset[int]) -> int:
    """Index of the zero byte that ends the string at @p position, or -1."""
    while position < len(data):
        code = data[position]
        if code == 0:
            return position
        position += 2 if code in two_byte_codes else 1
    return -1


def parse(data: bytes, start: int, two_byte_codes: frozenset[int]) -> TextTable:
    """Parse the table at @p start in @p data."""
    if start + 2 > len(data):
        raise ValueError("text table starts past the end of the data")
    first = int.from_bytes(data[start : start + 2], "little")
    if first == 0 or first % 2:
        raise ValueError(f"first offset 0x{first:X} is not a valid table size")
    count = first // 2
    entries = []
    end = first
    for index in range(count):
        offset = int.from_bytes(data[start + index * 2 : start + index * 2 + 2], "little")
        position = start + offset
        terminator = string_end(data, position, two_byte_codes)
        if offset < first or terminator < 0:
            raise ValueError(f"entry {index} has an invalid offset 0x{offset:X}")
        entries.append(TextEntry(index, offset, data[position:terminator], two_byte_codes))
        end = max(end, terminator + 1 - start)
    return TextTable(tuple(entries), end)


def decode(data: bytes, two_byte_codes: frozenset[int]) -> str:
    """Show game text as ASCII, with every other byte as {XX} (or {XX YY} for two-byte codes)."""
    out = []
    index = 0
    while index < len(data):
        code = data[index]
        if code in two_byte_codes and index + 1 < len(data):
            out.append(f"{{{code:02X} {data[index + 1]:02X}}}")
            index += 2
            continue
        if 0x20 <= code < 0x7F and code not in (0x7B, 0x7D):
            out.append(chr(code))
        else:
            out.append(f"{{{code:02X}}}")
        index += 1
    return "".join(out)
