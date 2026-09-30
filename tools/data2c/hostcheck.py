"""Check host output (data2c --host) against the region it came from.

The host C is compiled for x86-64. Each symbol is then read back from the
object through the host layout of the decomp's own declaration, and every
field is compared with the same field read from the PS1 data through the
PS1 layout: integers by value, pointers by the symbol they point at. The two
layouts are walked in step, so a field that moved, grew or changed type on
the host shows up as a difference.

This proves that game code compiled for the host, using the decomp's own
types, reads the values the PS1 build had.
"""
from __future__ import annotations

import pathlib
import struct
import subprocess

import clang.cindex as ci

from declarations import Declarations, TypeKind, clang_args, is_integer, is_signed, is_union, record_members
from elf import R_X86_64_64, STT_SECTION, ElfObject

HOST_TARGET = "--target=x86_64-unknown-linux-gnu"
EXAMPLES = 8


def host_args(version: str, includes) -> list[str]:
    return [HOST_TARGET] + [a for a in clang_args(version, includes) if not a.startswith("--target")]


def strip(t):
    return t.get_canonical()


# ----------------------------------------------------------------------------- reading values

class Ps1Reader:
    """Values in the region, as the PS1 build stores them."""

    def __init__(self, region, symbols):
        self.region, self.symbols = region, symbols

    def int(self, addr, size, signed):
        return self.region.word(addr, size, signed)

    def raw(self, addr, size):
        return self.region.bytes_at(addr, size)

    def ptr(self, addr):
        if addr in self.region.relocations:
            return self.region.relocations[addr]
        value = self.region.word(addr)
        if value == 0:
            return None
        if not self.region.from_object:
            found = self.symbols.resolve(value)
            if found:
                return found
        return ("raw", value)


class HostReader:
    """Values in the compiled host object, one symbol at a time."""

    def __init__(self, elf: ElfObject):
        self.elf = elf
        self.by_name = {s.name: s for s in elf.symbols if s.name and s.section and s.kind != STT_SECTION}
        self.relocs = {}
        for index in {s.section for s in self.by_name.values()}:
            if 0 < index < len(elf.sections):
                self.relocs[index] = {r.offset: r for r in elf.relocations(index)}
        self.section, self.base = None, 0

    def at(self, name) -> bool:
        sym = self.by_name.get(name)
        if sym is None:
            return False
        self.section, self.base = sym.section, sym.value
        self.data = self.elf.contents(sym.section)
        return True

    def int(self, off, size, signed):
        return int.from_bytes(self.data[self.base + off:self.base + off + size], "little", signed=signed)

    def raw(self, off, size):
        return self.data[self.base + off:self.base + off + size]

    def target(self, rel):
        """(name, offset) a relocation points at."""
        if rel.symbol.kind == STT_SECTION or not rel.symbol.name:
            # Section-relative: name the object the addend lands in.
            inside = [s for s in self.by_name.values() if s.section == rel.symbol.section and s.value <= rel.addend]
            sym = max(inside, key=lambda s: s.value)
            return sym.name, rel.addend - sym.value
        return rel.symbol.name, rel.addend

    def ptr(self, off):
        rel = self.relocs.get(self.section, {}).get(self.base + off)
        if rel is not None:
            if rel.type != R_X86_64_64:
                return ("reloc type", rel.type)
            return self.target(rel)
        value = self.int(off, 8, False)
        return None if value == 0 else ("raw", value)


# ----------------------------------------------------------------------------- comparing

class Compare:
    def __init__(self):
        self.fields = 0
        self.differences = []

    def differ(self, where, ps1, host):
        self.differences.append(f"{where}: PS1 {ps1}, host {host}")

    def value(self, pt, ht, pr, hr, pa, ha, where, count=None):
        pt, ht = strip(pt), strip(ht)
        k = pt.kind
        if is_integer(pt):
            self.fields += 1
            a = pr.int(pa, pt.get_size(), is_signed(pt))
            b = hr.int(ha, ht.get_size(), is_signed(ht))
            if a != b:
                self.differ(where, a, b)
        elif k == TypeKind.POINTER:
            self.fields += 1
            a, b = pr.ptr(pa), hr.ptr(ha)
            if a != b:
                self.differ(where, a, b)
        elif k == TypeKind.FLOAT:
            self.fields += 1
            if pr.raw(pa, 4) != hr.raw(ha, 4):
                self.differ(where, pr.raw(pa, 4).hex(), hr.raw(ha, 4).hex())
        elif k in (TypeKind.CONSTANTARRAY, TypeKind.INCOMPLETEARRAY):
            n = pt.element_count if k == TypeKind.CONSTANTARRAY else count
            pe, he = strip(pt.element_type), strip(ht.element_type)
            for i in range(n or 0):
                self.value(pe, he, pr, hr, pa + i * pe.get_size(), ha + i * he.get_size(), f"{where}[{i}]")
        elif k == TypeKind.RECORD:
            pf, hf = list(pt.get_fields()), list(ht.get_fields())
            if len(pf) != len(hf):
                self.differ(where, f"{len(pf)} fields", f"{len(hf)} fields")
                return
            pairs = list(zip(pf, hf))
            if is_union(pt):
                # The member data2c initialized: the widest on the PS1.
                first = next(m[1] for m in record_members(pt) if m[0] == "field")
                pairs = [p for p in pairs if p[0].get_field_offsetof() == first.get_field_offsetof()
                         and p[0].spelling == first.spelling][:1]
            for f, g in pairs:
                name = f"{where}.{f.spelling or '(anonymous)'}"
                po, ho = f.get_field_offsetof(), g.get_field_offsetof()
                if f.is_bitfield():
                    self.fields += 1
                    width = f.get_bitfield_width()
                    a = self.bits(pr, pa, po, width)
                    b = self.bits(hr, ha, ho, width)
                    if a != b:
                        self.differ(name, a, b)
                    continue
                self.value(f.type, g.type, pr, hr, pa + po // 8, ha + ho // 8, name)
        else:
            self.differ(where, f"unsupported {pt.spelling}", "")

    @staticmethod
    def bits(reader, base, offset_bits, width):
        first, last = offset_bits // 8, (offset_bits + width + 7) // 8
        v = int.from_bytes(reader.raw(base + first, last - first), "little")
        return (v >> (offset_bits % 8)) & ((1 << width) - 1)


def hostcheck(c_file: pathlib.Path, region, symbols, writer, sources, version: str, includes) -> bool:
    obj = c_file.with_suffix(".host.o")
    r = subprocess.run(["cc", "-std=gnu99", "-fno-common", "-fdata-sections", "-fno-builtin", "-w", "-O0",
                        "-c", str(c_file), "-o", str(obj)], capture_output=True, text=True)
    if r.returncode:
        print("HOSTCHECK: compile failed\n" + r.stderr[:3000])
        return False
    host = HostReader(ElfObject(obj))
    ps1 = Ps1Reader(region, symbols)
    wanted = set(writer.chosen) | {n for name in writer.chosen for n in symbols.names_at(symbols.address_of(name))}
    host_decls = Declarations(sources, wanted, host_args(version, includes))
    cmp = Compare()
    checked = missing = 0
    for name, (position, count) in writer.chosen.items():
        addr = symbols.address_of(name)
        names = symbols.names_at(addr)
        ps1_t = writer.decls.for_names(names)[position][0]
        host_candidates = host_decls.for_names(names)
        if position >= len(host_candidates) or not host.at(name):
            missing += 1
            continue
        cmp.value(ps1_t, host_candidates[position][0], ps1, host, addr, 0, name, count)
        checked += 1
    print(f"HOSTCHECK: {checked} symbols, {cmp.fields} fields compared, {len(cmp.differences)} differ"
          + (f", {missing} not found" if missing else ""))
    for d in cmp.differences[:EXAMPLES]:
        print(f"HOSTCHECK:   {d}")
    return not cmp.differences and not missing
