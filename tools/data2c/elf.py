"""Minimal reader for the little-endian ELF32 objects the PS1 toolchain writes.

data2c needs three things from an object file: a section's bytes, the symbols
defined in it, and its R_MIPS_32 relocations. That is small enough to read
directly, so the tool needs no ELF library.
"""
from __future__ import annotations

import pathlib
import struct
from dataclasses import dataclass

SHT_NOBITS = 8          # .bss-style section with no bytes in the file
SHT_REL = 9             # relocations without addends (MIPS keeps addends in place)
STT_SECTION = 3         # symbol that stands for a whole section
R_MIPS_32 = 2           # a full 32-bit address


@dataclass
class Section:
    name: str
    type: int
    offset: int          # file offset of the contents
    size: int
    link: int
    info: int            # for SHT_REL: index of the section the relocations apply to


@dataclass
class Symbol:
    name: str
    value: int           # offset within its section
    size: int
    kind: int            # STT_* type
    section: int         # section index, 0 when undefined


@dataclass
class Relocation:
    offset: int          # offset within the relocated section
    type: int
    symbol: Symbol


class ElfObject:
    def __init__(self, path: pathlib.Path | str):
        self.path = pathlib.Path(path)
        self.data = self.path.read_bytes()
        shoff, = struct.unpack_from("<I", self.data, 0x20)
        shentsize, shnum, shstrndx = struct.unpack_from("<HHH", self.data, 0x2E)
        raw = [struct.unpack_from("<IIIIIIIIII", self.data, shoff + i * shentsize) for i in range(shnum)]
        names_offset = raw[shstrndx][4]
        self.sections = [Section(self._cstr(names_offset + r[0]), r[1], r[4], r[5], r[6], r[7]) for r in raw]
        self.symbols = self._read_symbols()

    def _cstr(self, offset: int) -> str:
        return self.data[offset:self.data.index(b"\0", offset)].decode()

    def _read_symbols(self) -> list[Symbol]:
        symtab = next(s for s in self.sections if s.name == ".symtab")
        strtab = self.sections[symtab.link]
        out = []
        for i in range(symtab.size // 16):
            name, value, size, info, _other, shndx = struct.unpack_from("<IIIBBH", self.data, symtab.offset + i * 16)
            out.append(Symbol(self._cstr(strtab.offset + name), value, size, info & 0xF, shndx))
        return out

    def section_index(self, name: str) -> int:
        return next(i for i, s in enumerate(self.sections) if s.name == name)

    def contents(self, index: int) -> bytes:
        s = self.sections[index]
        if s.type == SHT_NOBITS:
            return bytes(s.size)
        return self.data[s.offset:s.offset + s.size]

    def relocations(self, index: int) -> list[Relocation]:
        """Relocations that apply to section `index`."""
        out = []
        for s in self.sections:
            if s.type != SHT_REL or s.info != index:
                continue
            for j in range(s.size // 8):
                offset, info = struct.unpack_from("<II", self.data, s.offset + j * 8)
                out.append(Relocation(offset, info & 0xFF, self.symbols[info >> 8]))
        return out
