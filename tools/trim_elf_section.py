#!/usr/bin/env python3
"""Trim verified zero padding from a section in a 32-bit little-endian ELF."""

import argparse
import struct
from pathlib import Path


ELF32_HEADER = struct.Struct("<16sHHIIIIIHHHHHH")
ELF32_SECTION = struct.Struct("<IIIIIIIIII")


def c_string(data: bytes, offset: int) -> str:
    end = data.index(0, offset)
    return data[offset:end].decode("ascii")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("elf", type=Path)
    parser.add_argument("section")
    parser.add_argument("count", type=lambda value: int(value, 0))
    args = parser.parse_args()

    data = bytearray(args.elf.read_bytes())
    header = ELF32_HEADER.unpack_from(data)
    ident = header[0]
    if ident[:6] != b"\x7fELF\x01\x01":
        raise SystemExit(f"{args.elf}: expected a 32-bit little-endian ELF")

    section_offset = header[6]
    section_entry_size = header[11]
    section_count = header[12]
    string_table_index = header[13]
    if section_entry_size != ELF32_SECTION.size:
        raise SystemExit(f"{args.elf}: unexpected section-header size {section_entry_size}")

    string_header_offset = section_offset + string_table_index * section_entry_size
    string_header = ELF32_SECTION.unpack_from(data, string_header_offset)
    strings = data[string_header[4] : string_header[4] + string_header[5]]

    for index in range(section_count):
        header_offset = section_offset + index * section_entry_size
        section = ELF32_SECTION.unpack_from(data, header_offset)
        if c_string(strings, section[0]) != args.section:
            continue

        file_offset = section[4]
        size = section[5]
        if args.count <= 0 or args.count > size:
            raise SystemExit(f"{args.elf}: invalid trim count {args.count} for {args.section} size {size}")
        tail = data[file_offset + size - args.count : file_offset + size]
        if any(tail):
            raise SystemExit(f"{args.elf}: refusing to trim nonzero bytes from {args.section}")

        struct.pack_into("<I", data, header_offset + 20, size - args.count)
        args.elf.write_bytes(data)
        return

    raise SystemExit(f"{args.elf}: section {args.section!r} not found")


if __name__ == "__main__":
    main()
