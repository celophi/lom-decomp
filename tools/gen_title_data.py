#!/usr/bin/env python3
"""Generate TITLE overlay initializer fragments from the original disc image.

The committed title.c describes the complete .rodata/.data layout. Creative
TIM pixels and game-authored save/menu records are supplied by generated,
gitignored fragments so the repository does not distribute those values.

Usage:
    python tools/gen_title_data.py
    python tools/gen_title_data.py --check
"""

import argparse
import importlib.util
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
DISC_PATH = REPO_ROOT / "disc" / "BIN" / "TITLE.BIN"
GEN_DIR = REPO_ROOT / "src" / "overlays" / "title" / "gen"

SEG_ROM_START = 0x1
SEG_ROM_END = 0xB2A5D
DATA_ROM_START = 0x2679
DATA_ROM_END = 0xB2A5D
DATA_VRAM_START = 0x800522E8
DATA_VRAM_END = 0x801026CC


class Fragment:
    def __init__(self, name: str, start: int, end: int, elem_size: int,
                 scalar: bool = False):
        self.name = name
        self.start = start
        self.end = end
        self.elem_size = elem_size
        self.scalar = scalar


# Only copyrighted/game-authored ranges need fragments. Mechanical tables,
# palettes, and zero-filled state are written directly in title.c.
FRAGMENTS = [
    Fragment("g_titleMenuTimImages", 0x800522F4, 0x8007FD2C, 4),
    Fragment("D_8007FD30", 0x8007FD30, 0x800A2F54, 4),
    Fragment("D_800A2F54", 0x800A2F54, 0x800AAF98, 4),
    Fragment("D_800AAF98", 0x800AAF98, 0x800AF43C, 4),
    Fragment("D_800AF43C", 0x800AF43C, 0x800B38E0, 4),
    Fragment("D_800B38E0", 0x800B38E0, 0x800B3EA4, 4),
    Fragment("D_800B3EA4", 0x800B3EA4, 0x800B62E8, 4),
    Fragment("D_800B62E8", 0x800B62E8, 0x800C352C, 4),
    Fragment("D_800C352C", 0x800C352C, 0x800D0770, 4),
    Fragment("D_800D0770", 0x800D0770, 0x800D93B4, 4),
    Fragment("D_800D93B4", 0x800D93B4, 0x800E95D8, 4),
    Fragment("D_800E95D8", 0x800E95D8, 0x800F22AC, 4),
    Fragment("g_frame_counter", 0x800F22AC, 0x800F22B0, 4, scalar=True),
    Fragment("D_800F22B0", 0x800F22B0, 0x800F97F8, 4),
    Fragment("D_800F98AC", 0x800F98AC, 0x800F98F4, 1),
    Fragment("D_800F98F4", 0x800F98F4, 0x800F993C, 1),
    Fragment("g_saveLayoutTable", 0x800F993C, 0x800F9BC4, 1),
    Fragment("g_saveSlotData", 0x800F9BC4, 0x800F9E84, 1),
    Fragment("g_menuLayoutTemplateDefault", 0x800F9E84, 0x800FE778, 4),
    Fragment("g_prim_rect_buf", 0x800FE778, 0x800FEF40, 1),
    Fragment("g_menuLayoutTemplateAlt", 0x800FEF40, 0x801021A0, 4),
    Fragment("g_subMenuLayoutDefault", 0x801021A0, 0x801023F0, 4),
    Fragment("g_subMenuLayoutContinue", 0x801023F0, 0x80102640, 4),
]

TIM_RANGES = [
    (0x800522F4, 0x8005A514),
    (0x8005A514, 0x8007FD28),
    (0x8007FD30, 0x800A2F50),
    (0x800A2F54, 0x800AAF94),
    (0x800AAF98, 0x800AF438),
    (0x800AF43C, 0x800B38DC),
    (0x800B38E0, 0x800B3EA0),
    (0x800B3EA4, 0x800B62E4),
    (0x800B62E8, 0x800C3528),
    (0x800C352C, 0x800D076C),
    (0x800D0770, 0x800D93B0),
    (0x800D93B4, 0x800E95D4),
    (0x800E95D8, 0x800F97F8),
]


def _load_decompress():
    path = REPO_ROOT / "tools" / "splat_ext" / "decompress.py"
    spec = importlib.util.spec_from_file_location("decompress", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.decompress


def extract_data_bytes() -> bytes:
    """Reproduce decompress_overlay's address space and return title .data."""
    raw = DISC_PATH.read_bytes()
    decompressed = _load_decompress()(raw[SEG_ROM_START:SEG_ROM_END])
    image = bytearray(SEG_ROM_START + len(decompressed))
    image[SEG_ROM_START:] = decompressed
    data = bytes(image[DATA_ROM_START:DATA_ROM_END])
    expected = DATA_VRAM_END - DATA_VRAM_START
    if len(data) != expected:
        sys.exit(f"ERROR: sliced {len(data)} bytes, expected {expected}")
    return data


def slice_vram(data: bytes, start: int, end: int) -> bytes:
    return data[start - DATA_VRAM_START:end - DATA_VRAM_START]


def little(data: bytes) -> int:
    return int.from_bytes(data, "little")


def validate_layout(data: bytes) -> None:
    """Check the non-generated schema and prove each artwork range is a TIM."""
    table = slice_vram(data, 0x800522E8, 0x800522F4)
    if [little(table[i:i + 4]) for i in range(0, 12, 4)] != [2, 0xC, 0x822C]:
        sys.exit("ERROR: unexpected title TIM offset table")

    if slice_vram(data, 0x8007FD2C, 0x8007FD30) != bytes((0, 0x10, 0x20, 0x10)):
        sys.exit("ERROR: unexpected cursor palette")

    for start, end, name in [
        (0x80102640, 0x801026CC, "runtime state"),
    ]:
        if any(slice_vram(data, start, end)):
            sys.exit(f"ERROR: {name} is not entirely zero-filled")

    for address, expected_end in TIM_RANGES:
        header = slice_vram(data, address, address + 8)
        magic = little(header[:4])
        flags = little(header[4:8])
        if magic != 0x10 or flags not in (2, 8, 9):
            sys.exit(
                f"ERROR: 0x{address:08X} is not a recognized TIM header "
                f"(magic=0x{magic:X}, flags=0x{flags:X})"
            )
        block_address = address + 8
        if flags & 8:
            block_address += little(slice_vram(data, block_address, block_address + 4))
        actual_end = block_address + little(
            slice_vram(data, block_address, block_address + 4)
        )
        if actual_end != expected_end:
            sys.exit(
                f"ERROR: TIM at 0x{address:08X} ends at 0x{actual_end:08X}, "
                f"expected 0x{expected_end:08X}"
            )

    for fragment in FRAGMENTS:
        size = fragment.end - fragment.start
        if size <= 0 or size % fragment.elem_size:
            sys.exit(f"ERROR: invalid extent for {fragment.name}: 0x{size:X}")
        if fragment.scalar and size != fragment.elem_size:
            sys.exit(f"ERROR: scalar {fragment.name} is 0x{size:X} bytes")


def render_fragment(fragment: Fragment, payload: bytes) -> str:
    header = (
        f"/* GENERATED by tools/gen_title_data.py for {fragment.name} - "
        "do not edit or commit. */"
    )
    values = []
    for offset in range(0, len(payload), fragment.elem_size):
        item = payload[offset:offset + fragment.elem_size]
        width = fragment.elem_size * 2
        values.append(f"0x{little(item):0{width}X}")

    if fragment.scalar:
        return f"{header}\n{values[0]}\n"

    per_line = 6 if fragment.elem_size == 4 else 12
    lines = [header]
    for index in range(0, len(values), per_line):
        lines.append("    " + ", ".join(values[index:index + per_line]) + ",")
    return "\n".join(lines) + "\n"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="validate ranges without writing fragments")
    args = parser.parse_args()

    data = extract_data_bytes()
    validate_layout(data)

    if args.check:
        generated_size = sum(fragment.end - fragment.start for fragment in FRAGMENTS)
        print(f"OK: TITLE .data is {len(data)} bytes")
        print(f"OK: {len(TIM_RANGES)} TIM images identified as copyrighted artwork")
        print(f"OK: {generated_size} game-authored bytes routed to ignored fragments")
        print("OK: mechanical tables/state remain committed C data")
        return

    GEN_DIR.mkdir(parents=True, exist_ok=True)
    for fragment in FRAGMENTS:
        payload = slice_vram(data, fragment.start, fragment.end)
        output = GEN_DIR / f"{fragment.name}.inc"
        output.write_text(render_fragment(fragment, payload), newline="\n")

    print(f"Wrote {len(FRAGMENTS)} fragments to {GEN_DIR.relative_to(REPO_ROOT)}")


if __name__ == "__main__":
    main()
