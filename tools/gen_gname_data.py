#!/usr/bin/env python3
"""Generate the gitignored initializer fragments for GNAME's .data tables from
the original (copyrighted) disc image.

The GNAME overlay is stored compressed on disc. splat's decompress_overlay step
inflates it and places the decompressed image at the segment rom_start so
subsegment offsets index correctly. We reproduce that here, then slice the
.data region into one fragment per named symbol (from gname_symbol_addrs.txt)
and format each fragment as a typed C initializer matching the declaration in
gname_data.c (scalars, offset arrays, and record structs).

The committed gname_data.c holds the schema (the typed declarations) and
#includes these fragments. The fragments hold ONLY data values and are
gitignored, keeping copyrighted content out of source control while letting the
linker build the section by compiling C - not by including a raw blob. If a
struct layout is wrong, the compiled bytes diverge from the original and the
overlay stops matching.

Usage:
    python tools/gen_gname_data.py            # write the fragments
    python tools/gen_gname_data.py --check    # verify against extracted asset
"""

import argparse
import importlib.util
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

# --- Overlay/segment geometry (from config/overlays/GNAME.BIN.yaml) ----------
DISC_PATH = REPO_ROOT / "disc" / "BIN" / "GNAME.BIN"
SEG_ROM_START = 0x1       # segment 'start' in the yaml
SEG_ROM_END = 0xF8D9      # trailing position marker in the yaml
DATA_ROM_START = 0x2C99   # .data subsegment offset
DATA_ROM_END = 0xF7B1     # following subsegment (.bss) offset
DATA_VRAM_START = 0x80142C98
DATA_VRAM_END = 0x8014F7B0  # first .bss symbol (g_custom_name_buf)

SYMBOL_FILE = REPO_ROOT / "config" / "symbols" / "gname_symbol_addrs.txt"
GEN_DIR = REPO_ROOT / "src" / "overlays" / "gname" / "gen"
REFERENCE_ASSET = REPO_ROOT / "assets" / "gname_data.databin.bin"

_SYM_RE = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;")


def _le(b: bytes) -> int:
    return int.from_bytes(b, "little")


# --- Per-element formatters --------------------------------------------------
# Each takes one element's bytes and returns the C initializer text for it.

def _fmt_u32(b):
    return f"0x{_le(b):08X}"

def _fmt_s32(b):
    return f"0x{_le(b):08X}"

def _fmt_u16(b):
    return f"0x{_le(b):04X}"

def _fmt_u8(b):
    return f"0x{b[0]:02X}"

def _fmt_glyphinfo(b):  # GlyphInfo: u8 u,v,w,h; u32 clut
    return (f"{{ 0x{b[0]:02X}, 0x{b[1]:02X}, 0x{b[2]:02X}, 0x{b[3]:02X}, "
            f"0x{_le(b[4:8]):X} }}")

def _fmt_tabcursor(b):  # TabCursorEntry: x:9, sprite_idx:7, u8 y, u8 glyph
    w = _le(b[0:2])
    return f"{{ {w & 0x1FF}, {(w >> 9) & 0x7F}, 0x{b[2]:02X}, 0x{b[3]:02X} }}"

def _fmt_glyphseq(b):  # GlyphSeqEntry: u32 id, u32 xy
    return f"{{ 0x{_le(b[0:4]):X}, 0x{_le(b[4:8]):08X} }}"

def _fmt_slot(b):  # GlyphAppendAnimSlot: u8 x,y,glyph,pad
    return f"{{ 0x{b[0]:02X}, 0x{b[1]:02X}, 0x{b[2]:02X}, 0x{b[3]:02X} }}"

def _fmt_frame(b):  # GlyphAppendAnimFrame: GlyphAppendAnimSlot slots[3]
    slots = ", ".join(_fmt_slot(b[i * 4:i * 4 + 4]) for i in range(3))
    return f"{{ {{ {slots} }} }}"


# --- Table specs -------------------------------------------------------------
# elem: bytes per element; fmt: element formatter; per_line: elements per line;
# scalar: emit a bare value (no braces/commas) for a single-element symbol.
class Spec:
    def __init__(self, elem, fmt, per_line=1, scalar=False):
        self.elem = elem
        self.fmt = fmt
        self.per_line = per_line
        self.scalar = scalar


SPECS = {
    "g_panel_char_offsets":       Spec(4, _fmt_u32, per_line=4),
    "g_kanji_cat_names_offset":   Spec(4, _fmt_s32, per_line=4),   # s32[2]: offset + boundary word
    "g_kanji_cat_entries":        Spec(4, _fmt_u32, per_line=8),
    "g_glyph_table":              Spec(8, _fmt_glyphinfo),
    "g_tab_cursor_pos":           Spec(4, _fmt_tabcursor),
    "g_tab_cursor_entries":       Spec(4, _fmt_tabcursor),
    "g_kanji_entry_offsets":      Spec(4, _fmt_u32, per_line=8),
    "g_panel_data_base":          Spec(4, _fmt_u32, scalar=True),
    "g_panel_tbl_off":            Spec(4, _fmt_u32, scalar=True),
    "g_kanji_panel_offset":       Spec(4, _fmt_u32, scalar=True),
    "g_history_names_off":        Spec(4, _fmt_u32, scalar=True),
    "g_random_names_off":         Spec(4, _fmt_u32, scalar=True),
    "g_panel_record_offsets":     Spec(2, _fmt_u16, per_line=12),
    "g_name_entry_tim":           Spec(1, _fmt_u8, per_line=12),   # TIM image blob
    "g_layout_sprite_sequence":   Spec(8, _fmt_glyphseq),
    "g_glyph_append_anim_frames": Spec(12, _fmt_frame),            # 7 frames + 4 tail bytes
}


def _load_decompress():
    ext = REPO_ROOT / "tools" / "splat_ext" / "decompress.py"
    spec = importlib.util.spec_from_file_location("decompress", ext)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod.decompress


def extract_data_bytes() -> bytes:
    """Reproduce splat's decompressed image and slice the .data range."""
    decompress = _load_decompress()
    rom = DISC_PATH.read_bytes()
    decompressed = decompress(rom[SEG_ROM_START:SEG_ROM_END])
    image = bytearray(SEG_ROM_START + len(decompressed))
    image[SEG_ROM_START:] = decompressed
    data = bytes(image[DATA_ROM_START:DATA_ROM_END])
    expected = DATA_VRAM_END - DATA_VRAM_START
    if len(data) != expected:
        sys.exit(f"ERROR: sliced {len(data)} bytes, expected {expected}")
    return data


def region_symbols():
    """Return [(name, vram)] for every symbol in the .data region, sorted."""
    syms = []
    for line in SYMBOL_FILE.read_text().splitlines():
        m = _SYM_RE.match(line)
        if not m:
            continue
        addr = int(m.group(2), 16)
        if DATA_VRAM_START <= addr < DATA_VRAM_END:
            syms.append((m.group(1), addr))
    syms.sort(key=lambda t: t[1])
    if not syms or syms[0][1] != DATA_VRAM_START:
        sys.exit(f"ERROR: region does not start at 0x{DATA_VRAM_START:X}")
    return syms


def slices(data: bytes):
    """Yield (name, vram, size, bytes) partitioning the region by symbol."""
    syms = region_symbols()
    bounds = [addr for _, addr in syms] + [DATA_VRAM_END]
    for i, (name, addr) in enumerate(syms):
        start = addr - DATA_VRAM_START
        end = bounds[i + 1] - DATA_VRAM_START
        yield name, addr, end - start, data[start:end]


def render_fragment(name: str, spec: Spec, data: bytes) -> str:
    """Format a table's bytes as a typed C initializer fragment. Returns the
    fragment text; the caller also receives any trailing bytes (bytes that do
    not fill a whole element) via the second return value."""
    header = f"/* GENERATED by tools/gen_gname_data.py for {name} - do not edit or commit. */"
    count = len(data) // spec.elem
    tail = data[count * spec.elem:]

    if spec.scalar:
        # A bare value expression for `T name =\n#include ...\n;`.
        if count != 1:
            sys.exit(f"ERROR: {name} marked scalar but has {count} elements")
        return f"{header}\n{spec.fmt(data)}\n", tail

    items = [spec.fmt(data[i * spec.elem:(i + 1) * spec.elem]) for i in range(count)]
    lines = [header]
    for i in range(0, len(items), spec.per_line):
        chunk = items[i:i + spec.per_line]
        lines.append("    " + ", ".join(chunk) + ",")
    return "\n".join(lines) + "\n", tail


def render_tail(name: str, tail: bytes) -> str:
    header = f"/* GENERATED by tools/gen_gname_data.py for {name} tail - do not edit or commit. */"
    body = ", ".join(f"0x{b:02X}" for b in tail)
    return f"{header}\n    {body},\n"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="verify extracted bytes match the extracted asset")
    args = parser.parse_args()

    data = extract_data_bytes()

    if args.check:
        table_list = list(slices(data))
        total = 0
        for name, addr, size, _ in table_list:
            spec = SPECS.get(name)
            note = ""
            if spec:
                count = size // spec.elem
                tailn = size - count * spec.elem
                note = f"{count} x {spec.elem}" + (f" + {tailn} tail" if tailn else "")
            print(f"  0x{addr:08X}  {size:6d}  {name:30s} {note}")
            total += size
        if total != len(data):
            sys.exit(f"ERROR: tables sum to {total}, region is {len(data)} bytes")
        note = ""
        if REFERENCE_ASSET.exists():
            if data != REFERENCE_ASSET.read_bytes():
                sys.exit(f"MISMATCH vs {REFERENCE_ASSET.name}")
            note = f"; bytes match {REFERENCE_ASSET.name}"
        print(f"OK: {total} bytes across {len(table_list)} tables{note}")
        return

    GEN_DIR.mkdir(parents=True, exist_ok=True)
    count = 0
    for name, _, _, chunk in slices(data):
        spec = SPECS.get(name)
        if spec is None:
            sys.exit(f"ERROR: no spec for symbol {name}")
        frag, tail = render_fragment(name, spec, chunk)
        (GEN_DIR / f"{name}.inc").write_text(frag, newline="\n")
        count += 1
        if tail:
            (GEN_DIR / f"{name}_tail.inc").write_text(render_tail(name, tail), newline="\n")
            count += 1
    print(f"Wrote {count} fragments to {GEN_DIR.relative_to(REPO_ROOT)}")


if __name__ == "__main__":
    main()
