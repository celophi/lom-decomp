"""One contiguous .data region of an image, as the linker places it.

A region comes from one of splat's two kinds of data file:

- A databin: a .s holding a single `.incbin` of an extracted blob. The bytes
  are final; which words are pointers has to be worked out from their values.
- Data assembly: a .s with `.word func_x` style directives. Once assembled,
  its relocations say exactly which words are pointers and to what, so no
  guessing is needed. The addends sit in place (MIPS REL relocations).
"""
from __future__ import annotations

import pathlib
import re
import struct
from dataclasses import dataclass, field

from elf import ElfObject, R_MIPS_32, STT_SECTION
from symbols import SymbolTable

INCBIN = re.compile(r'^\.incbin\s+"([^"]+)"', re.M)
DLABEL = re.compile(r"^dlabel\s+(\w+)", re.M)
# spimdisasm comments each item with its ROM offset and address: /* 74919 800C4588 ... */
MARKER = ".NON_MATCHING"
ITEM_ADDRESS = re.compile(r"/\*\s+[0-9A-F]+\s+([0-9A-F]{8})\s")


@dataclass
class DataRegion:
    source: str                     # the .s it came from, for messages
    start: int                      # address of the first byte
    data: bytes                     # bytes as stored; pointer words hold their addend
    linked: bytes                   # bytes after linking, to verify against
    relocations: dict = field(default_factory=dict)   # address -> (target, addend)
    from_object: bool = False       # pointers are known from relocations

    @property
    def end(self) -> int:
        return self.start + len(self.data)

    def word(self, addr: int, size: int = 4, signed: bool = False) -> int:
        off = addr - self.start
        return int.from_bytes(self.data[off:off + size], "little", signed=signed)

    def bytes_at(self, addr: int, size: int) -> bytes:
        off = addr - self.start
        return self.data[off:off + size]


def load_databin(repo: pathlib.Path, asm: str, symbols: SymbolTable) -> DataRegion:
    """A databin .s: one dlabel naming the blob, one .incbin of its bytes."""
    text = (repo / asm).read_text()
    blobs, labels = INCBIN.findall(text), DLABEL.findall(text)
    if len(blobs) != 1 or len(labels) != 1:
        raise SystemExit(f"{asm}: expected one .incbin and one dlabel")
    start = symbols.address_of(labels[0])
    if start is None:
        raise SystemExit(f"{asm}: no address for {labels[0]}")
    symbols.preferred.add(labels[0])
    data = (repo / blobs[0]).read_bytes()
    return DataRegion(asm, start, data, data)


def region_start(repo: pathlib.Path, asm: str, labels, symbols: SymbolTable) -> int:
    """Where assembled data starts: from the labels whose addresses the symbol
    files (or the names themselves) give, falling back to the address in
    spimdisasm's first item comment. Disagreement is an error."""
    starts = {symbols.address_of(s.name) - s.value for s in labels if symbols.address_of(s.name) is not None}
    if len(starts) > 1:
        raise SystemExit(f"{asm}: its labels disagree about where the data starts: "
                         + ", ".join(f"{a:#x}" for a in sorted(starts)))
    if starts:
        return starts.pop()
    first = ITEM_ADDRESS.search((repo / asm).read_text())
    if not first:
        raise SystemExit(f"{asm}: no label with a known address and no address comment")
    return int(first.group(1), 16)


def load_assembled(repo: pathlib.Path, obj: str, asm: str, symbols: SymbolTable) -> DataRegion:
    """Assembled data: bytes, labels and relocations from the object file."""
    elf = ElfObject(repo / obj)
    index = elf.section_index(".data")
    data = elf.contents(index)
    # splat's `nonmatching` macro also adds NAME.NON_MATCHING markers; nothing
    # links against those.
    labels = [s for s in elf.symbols
              if s.section == index and s.name and s.kind != STT_SECTION and not s.name.endswith(MARKER)]
    start = region_start(repo, asm, labels, symbols)

    # The object's own labels are authoritative for names inside the region.
    for sym in labels:
        symbols.define(sym.name, start + sym.value)
        symbols.preferred.add(sym.name)

    relocations = {}
    linked = bytearray(data)
    for rel in elf.relocations(index):
        if rel.type != R_MIPS_32:
            raise SystemExit(f"{obj}: relocation type {rel.type} at {rel.offset:#x}")
        addend, = struct.unpack_from("<i", data, rel.offset)
        if rel.symbol.kind == STT_SECTION:
            # Relative to the section itself: name the label the address lands on.
            found = symbols.resolve(start + addend)
            if found is None:
                raise SystemExit(f"{obj}: section-relative relocation at {rel.offset:#x} has no label")
            target, addend = found
        else:
            target = rel.symbol.name
        target_addr = symbols.address_of(target)
        if target_addr is None:
            raise SystemExit(f"{obj}: no address for relocation target {target}")
        relocations[start + rel.offset] = (target, addend)
        struct.pack_into("<I", linked, rel.offset, (target_addr + addend) & 0xFFFFFFFF)
    return DataRegion(asm, start, data, bytes(linked), relocations, from_object=True)
