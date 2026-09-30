"""Check generated C against the region it came from, without the PS1 toolchain.

The C is compiled for mipsel with Clang, one section per variable. Each
section is placed at its symbol's real address, its relocations are applied
with the real target addresses, and the result must equal the region's linked
bytes. This proves the C holds the right bytes and pointers; the PS1 build's
own check (`make DATA_AS_C=1 verify-bins`) also proves GCC 2.8 lays it out the
same way.
"""
from __future__ import annotations

import pathlib
import struct
import subprocess

from elf import R_MIPS_32, ElfObject

GENERATED_PREFIXES = ("d2c_gap_", "d2c_cluster_")   # names ending in their address


def address_in_name(name: str) -> int:
    return int(name.rsplit("_", 1)[1], 16)


def verify(c_file: pathlib.Path, region, symbols) -> bool:
    obj_path = c_file.with_suffix(".verify.o")
    # -fno-builtin: names like printf are the game's own library code here.
    r = subprocess.run(["clang", "--target=mipsel-unknown-linux-gnu", "-fno-common", "-fdata-sections",
                        "-fno-builtin", "-O0", "-c", str(c_file), "-o", str(obj_path)],
                       capture_output=True, text=True)
    if r.returncode:
        print("VERIFY: compile failed\n" + r.stderr[:3000])
        return False
    elf = ElfObject(obj_path)
    size = len(region.linked)

    def where(name):
        if name.startswith(GENERATED_PREFIXES):
            return address_in_name(name)
        return symbols.address.get(name)

    # Each section's address follows from any named symbol defined in it.
    base = {}
    for sym in elf.symbols:
        addr = where(sym.name) if sym.name else None
        if 0 < sym.section < len(elf.sections) and sym.section not in base and addr is not None \
                and region.start <= addr < region.end:
            base[sym.section] = addr - sym.value

    image = bytearray(size)
    placed = bytearray(size)
    for index, addr in base.items():
        content = elf.contents(index)
        at = addr - region.start
        if at < 0 or at + len(content) > size:
            print(f"VERIFY: section {elf.sections[index].name} lands outside the region")
            continue
        image[at:at + len(content)] = content
        placed[at:at + len(content)] = b"\x01" * len(content)

    applied = 0
    for index, addr in base.items():
        for rel in elf.relocations(index):
            if rel.type != R_MIPS_32:
                print(f"VERIFY: unexpected relocation type {rel.type}")
                return False
            target = where(rel.symbol.name)
            if target is None and rel.symbol.section in base:
                target = base[rel.symbol.section] + rel.symbol.value
            if target is None:
                print(f"VERIFY: no address for relocation target {rel.symbol.name}")
                return False
            at = addr + rel.offset - region.start
            value, = struct.unpack_from("<I", image, at)
            struct.pack_into("<I", image, at, (value + target) & 0xFFFFFFFF)
            applied += 1

    missing = size - sum(placed)
    diffs = [i for i in range(size) if image[i] != region.linked[i]]
    print(f"VERIFY: {applied} relocations applied, {missing} bytes not placed, {len(diffs)} bytes differ")
    if missing:
        first = next(i for i in range(size) if not placed[i])
        print(f"VERIFY: first unplaced byte at {region.start + first:#x}")
    if diffs:
        i = diffs[0]
        print(f"VERIFY: first difference at {region.start + i:#x}: got {image[i]:#x}, want {region.linked[i]:#x}")
        return False
    if missing:
        return False
    print("VERIFY: generated C reproduces the region exactly")
    return True
