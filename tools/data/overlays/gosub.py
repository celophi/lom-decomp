#!/usr/bin/env python3
"""Export GOSUB's workshop and companion-list resources.

The build links one data blob and a short rodata blob of equipment ranges.
Readers return Parts; the byte maps cover both inputs. JP text uses CLOAD's
character chart. The UI glyph format and preview writer are shared with GOLEM.

Two symbols point inside resources: the portrait offsets follow their count
word, and the font symbol points at a TIM's palette. Its pixels extend into
the unused prefix of the item-color table, just as the C upload reads them.

Example:

    python3 -m tools.data.overlays.gosub --version us assets/exports/us/overlays/gosub
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import shutil
import struct
import sys
import tempfile

from tools.data.formats.psx_tim import TimImage, parse_embedded_tim, parse_tim
from tools.data.overlays import cload, icon_set, png, splat_config, symbols, text_table
from tools.data.overlays.card_data import Chart
from tools.data.overlays.glyph_texture import GLYPH_RECORD, Glyph, Image
from tools.data.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[3]
OVERLAY_CONFIG = "overlays/GOSUB.BIN.yaml"
SYMBOL_FILE = "symbols/gosub_symbol_addrs.txt"
HEADER = REPO_ROOT / "src/overlays/gosub/internal/gosub_internal.h"
GLYPH_COUNT = 82
SECTION_COUNT = 12
COLOR_FIRST = 96
COLOR_END = 133
FONT_PIXEL_OFFSET = 44
FONT_WIDTH_WORDS = 16
FONT_HEIGHT = 16
FONT_TIM_PREFIX = 20
PORTRAIT_GOLEM_FIRST = 65
PORTRAIT_EGG_FIRST = 72


@dataclass(frozen=True)
class GosubSymbols:
    """Resource boundaries; the group arrays live in a separate rodata blob."""

    image: int
    text: int
    portraits: int
    font: int
    colors: int
    glyphs: int
    bss: int
    group_firsts: int
    group_counts: int

    @classmethod
    def load(cls, path: Path) -> GosubSymbols:
        named = symbols.load(path)
        missing = [name for name in SYMBOL_NAMES.values() if name not in named]
        if missing:
            raise ValueError(f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                             "update SYMBOL_NAMES in tools/data/overlays/gosub.py.")
        return cls(**{key: named[name] for key, name in SYMBOL_NAMES.items()})


SYMBOL_NAMES = {
    "image": "g_gosub_image_archive", "text": "g_gosub_text_archive",
    "portraits": "g_gosub_portrait_archive", "font": "g_gosub_font_texture",
    "colors": "g_gosub_item_colors", "glyphs": "g_gosub_glyph_metrics",
    "bss": "g_gosub_frame_parity", "group_firsts": "g_gosub_equipment_type_firsts",
    "group_counts": "g_gosub_equipment_type_counts",
}
DATA_KEYS = ("image", "text", "portraits", "font", "colors", "glyphs")


@dataclass(frozen=True)
class Inputs:
    version: str
    config: Path
    assets: Path


@dataclass(frozen=True)
class Table:
    address: int
    values: dict[str, object]
    raw: bytes


@dataclass(frozen=True)
class TextArchive:
    address: int
    raw: bytes
    sections: tuple[dict[str, object], ...]


@dataclass(frozen=True)
class Portraits:
    address: int
    raw: bytes
    icons: icon_set.IconSet


@dataclass(frozen=True)
class Font:
    address: int
    tim: TimImage


def read_bytes(blob: Blob, address: int, end: int, what: str) -> bytes:
    start, stop = blob.offset(address), blob.offset(end)
    if not 0 <= start <= stop <= len(blob.data):
        raise ValueError(f"{what} is outside {blob.file_name}")
    return blob.data[start:stop]


def enum_names(type_name: str) -> dict[int, str]:
    source = HEADER.read_text(encoding="ascii")
    match = re.search(r"typedef enum\s*\{([^}]+)\}\s*" + type_name + ";", source)
    if match is None:
        raise ValueError(f"{HEADER} has no {type_name}")
    return {int(value): name for name, value in re.findall(r"(GOSUB_\w+)\s*=\s*(\d+)", match[1])}


# ---------------------------------------------------------------------------
# Reading the resources


def read_glyphs(blob: Blob[GosubSymbols]) -> tuple[Part, tuple[Glyph, ...]]:
    address = blob.symbols.glyphs
    raw = read_bytes(blob, address, address + GLYPH_COUNT * GLYPH_RECORD.size, "glyph metrics")
    glyphs = tuple(Glyph(*values) for values in GLYPH_RECORD.iter_unpack(raw))
    values = {"record_format": GLYPH_RECORD.format,
              "entries": [{"index": i, **glyph.document()} for i, glyph in enumerate(glyphs)]}
    start = blob.offset(address)
    return Part("glyph metrics", start, start + len(raw), "tables/glyph_metrics.yaml", Table(address, values, raw)), glyphs


def read_image(blob: Blob[GosubSymbols], glyphs: tuple[Glyph, ...]) -> list[Part]:
    address = blob.symbols.image
    raw = read_bytes(blob, address, blob.symbols.text, "UI TIM")
    tim = parse_embedded_tim(raw, trailing_duplicate_word=True)
    if tim.pixel_mode != 0 or tim.clut is None or not tim.clut.payload or len(tim.clut.payload) % 32:
        raise ValueError("UI TIM must contain 4-bit pixels and complete 16-color palettes")
    for index, glyph in enumerate(glyphs):
        if glyph.u + glyph.width > tim.pixels.width_words * 4 or glyph.v + glyph.height > tim.pixels.height:
            raise ValueError(f"glyph {index} lies outside the UI TIM")
    start = blob.offset(address)
    end = start + len(tim.to_bytes())
    return [Part("UI TIM", start, end, "image/image.yaml", Image(address, tim, glyphs)),
            Part("UI trailing word", end, end + 4, note="Repeats the TIM's last word.")]


def read_text(blob: Blob[GosubSymbols], chart: Chart | None) -> Part:
    address = blob.symbols.text
    # The word before the portrait symbol is the portrait count, not text.
    raw = read_bytes(blob, address, blob.symbols.portraits - 4, "text archive")
    header_size = (SECTION_COUNT + 1) * 4
    if len(raw) < header_size or struct.unpack_from("<I", raw)[0] != SECTION_COUNT:
        raise ValueError("text archive must contain twelve sections")
    offsets = struct.unpack_from("<" + "I" * SECTION_COUNT, raw, 4)
    if offsets[0] != header_size or list(offsets) != sorted(set(offsets)) or offsets[-1] >= len(raw):
        raise ValueError("invalid text section offsets")
    if blob.version == "jp" and chart is None:
        raise ValueError("JP GOSUB text needs CLOAD's character chart")
    names = enum_names("GosubTextSection")
    messages = enum_names("GosubMessage")
    sections = []
    for index, (start, end) in enumerate(zip(offsets, (*offsets[1:], len(raw)))):
        symbol = names[index]
        name = symbol.removeprefix("GOSUB_TEXT_").lower()
        parsed = text_table.parse(raw[start:end], 0, text_table.TWO_BYTE_CODES[blob.version])
        entries = []
        for entry in parsed.entries:
            value = {"index": entry.index, "offset": f"0x{entry.offset:X}",
                     "text": chart.decode(entry.data) if chart else entry.text, "bytes": entry.data.hex(" ")}
            if name == "messages" and entry.index in messages:
                value["symbol"] = messages[entry.index]
            entries.append(value)
        sections.append({"index": index, "symbol": symbol, "name": name, "offset": f"0x{start:X}",
                         "address": hex_address(address + start), "entries": entries})
    start = blob.offset(address)
    return Part("text archive", start, start + len(raw), "text/archive.yaml", TextArchive(address, raw, tuple(sections)))


def read_portraits(blob: Blob[GosubSymbols]) -> list[Part]:
    address = blob.symbols.portraits - 4
    raw = read_bytes(blob, address, blob.symbols.font - FONT_TIM_PREFIX, "portrait archive")
    icons = icon_set.parse(raw, 0)
    start = blob.offset(address)
    end = start + icons.size
    content = Portraits(address, raw[:icons.size], icons)
    parts = [Part("portraits", start, end, "portraits/portraits.yaml", content)]
    if raw[icons.size:] == raw[icons.size - 4:icons.size]:
        parts.append(Part("portrait trailing word", end, end + 4, note="Repeats the last portrait word."))
    return parts


def read_font(blob: Blob[GosubSymbols]) -> Part:
    """Recover the complete TIM around the palette symbol, including the overlapping pixels."""
    address = blob.symbols.font - FONT_TIM_PREFIX
    end = blob.symbols.font + FONT_PIXEL_OFFSET + FONT_WIDTH_WORDS * FONT_HEIGHT * 2
    raw = read_bytes(blob, address, end, "font TIM")
    tim = parse_tim(raw)
    if tim.pixel_mode != 0 or tim.clut is None or len(tim.clut.payload) != 32:
        raise ValueError("font TIM must have one 16-color palette")
    if (tim.pixels.width_words, tim.pixels.height) != (FONT_WIDTH_WORDS, FONT_HEIGHT):
        raise ValueError("font TIM dimensions differ from the upload rectangle")
    start = blob.offset(address)
    return Part("font TIM", start, start + len(raw), "font/image.yaml", Font(address, tim))


def read_colors(blob: Blob[GosubSymbols], text: TextArchive) -> Part:
    address = blob.symbols.colors + COLOR_FIRST
    raw = read_bytes(blob, address, blob.symbols.colors + COLOR_END, "item color indices")
    colors = next(section["entries"] for section in text.sections if section["name"] == "color_names")
    entries = []
    for index, color in enumerate(raw, COLOR_FIRST):
        if color >= len(colors):
            raise ValueError(f"item {index} has an invalid color-name index {color}")
        entries.append({"item_id": index, "color_index": color, "color_name": colors[color]["text"]})
    values = {"entries": entries, "note": "Only color-material item ids use this map. Its unused prefix overlaps the font pixels."}
    start = blob.offset(address)
    return Part("item color indices", start, start + len(raw), "tables/item_colors.yaml", Table(address, values, raw))


def read_blob(blob: Blob[GosubSymbols], chart: Chart | None = None) -> list[Part]:
    glyph_part, glyphs = read_glyphs(blob)
    parts = read_image(blob, glyphs)
    text = read_text(blob, chart)
    parts.append(text)
    parts += read_portraits(blob)
    parts.append(read_font(blob))
    parts.append(read_colors(blob, text.content))
    parts.append(glyph_part)
    return cover_gaps(blob, parts)


def read_groups(blob: Blob[GosubSymbols]) -> list[Part]:
    names = blob.symbols
    first_raw = read_bytes(blob, names.group_firsts, names.group_firsts + 12, "equipment first indices")
    count_raw = read_bytes(blob, names.group_counts, names.group_counts + 12, "equipment counts")
    firsts = struct.unpack("<3i", first_raw)
    counts = struct.unpack("<3i", count_raw)
    entries = [{"kind": kind, "first_index": first, "count": count}
               for kind, first, count in zip(("weapon", "armor", "instrument"), firsts, counts)]
    values = {"entries": entries, "text_table": "../text/equipment_types.yaml"}
    content = Table(names.group_firsts, values, first_raw + count_raw)
    return cover_gaps(blob, [Part("equipment groups", blob.offset(names.group_firsts),
                                  blob.offset(names.group_counts) + 12, "tables/equipment_groups.yaml", content)])


# ---------------------------------------------------------------------------
# Writing


@write_part.register
def _write_table(content: Table, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), **content.values, "bytes": content.raw.hex(" ")})


@write_part.register
def _write_text(content: TextArchive, path: Path) -> None:
    sections = [{key: section[key] for key in ("index", "symbol", "offset")} | {"file": f"{section['name']}.yaml"}
                for section in content.sections]
    dump_yaml(path, {"address": hex_address(content.address), "file": "archive.bin", "sections": sections})
    (path.parent / "archive.bin").write_bytes(content.raw)
    for section in content.sections:
        dump_yaml(path.parent / f"{section['name']}.yaml", section)


@write_part.register
def _write_portraits(content: Portraits, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    (path.parent / "archive.bin").write_bytes(content.raw)
    entries = []
    for icon in content.icons.icons:
        group = "pet" if icon.index < PORTRAIT_GOLEM_FIRST else "golem" if icon.index < PORTRAIT_EGG_FIRST else "egg"
        file = f"{icon.index:02d}.png"
        png.write_rgba(path.parent / file, icon_set.ICON_SIZE, icon_set.ICON_SIZE, icon.rgba())
        entries.append({"index": icon.index, "group": group, "offset": f"0x{icon.offset:X}", "file": file,
                        "palette": [f"0x{value:04X}" for value in icon.palette]})
    dump_yaml(path, {"address": hex_address(content.address), "file": "archive.bin", "entries": entries,
                     "note": "Offsets are relative to the count word, four bytes before g_gosub_portrait_archive. "
                             "PNG colors use the stored palette; zero is transparent and other colors are opaque."})


@write_part.register
def _write_font(content: Font, path: Path) -> None:
    tim = content.tim
    palette = struct.unpack("<16H", tim.clut.payload)
    colors = [icon_set.bgr555_to_rgba(value) for value in palette]
    pixels = [colors[nibble] for byte in tim.pixels.payload for nibble in (byte & 15, byte >> 4)]
    path.parent.mkdir(parents=True, exist_ok=True)
    (path.parent / "image.tim").write_bytes(tim.to_bytes())
    png.write_rgba(path.parent / "image.png", tim.pixels.width_words * 4, tim.pixels.height, pixels)
    dump_yaml(path, {"address": hex_address(content.address), "file": "image.tim", "preview": "image.png",
                     "stored_layout": tim.metadata(), "palette": [f"0x{value:04X}" for value in palette],
                     "note": "The font symbol points at the palette, twenty bytes into this TIM. "
                             "The 512 pixel bytes include 92 bytes from the unused item-color prefix. "
                             "Runtime upload: pixels at (320, 240), palette at (336, 255). "
                             "Zero is transparent in the PNG; other colors are opaque."})


# ---------------------------------------------------------------------------
# Putting it together


def load_inputs(inputs: Inputs) -> tuple[Blob[GosubSymbols], Blob[GosubSymbols]]:
    names = GosubSymbols.load(inputs.config / SYMBOL_FILE)
    files = splat_config.data_files(inputs.config / OVERLAY_CONFIG, inputs.assets)

    def load(address: int, what: str) -> Blob:
        source = splat_config.file_containing(files, address, what)
        if not source.path.exists():
            raise ValueError(f"{source.path} is missing; run make splat first")
        raw = source.path.read_bytes()
        if len(raw) != source.end - source.start:
            raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
        return Blob(raw, source.start, source.path.name, inputs.version, names)

    data = load(names.image, SYMBOL_NAMES["image"])
    groups = load(names.group_firsts, SYMBOL_NAMES["group_firsts"])
    previous = data.address - 1
    for key in DATA_KEYS:
        address = getattr(names, key)
        if not data.address <= address < data.address + len(data.data):
            raise ValueError(f"{SYMBOL_NAMES[key]} is outside {data.file_name}")
        if address <= previous:
            raise ValueError(f"{SYMBOL_NAMES[key]} is out of resource order")
        previous = address
    if names.bss != data.address + len(data.data):
        raise ValueError("GOSUB data must end at g_gosub_frame_parity, where BSS begins")
    if names.group_counts != names.group_firsts + 12:
        raise ValueError("equipment group arrays must be adjacent")
    return data, groups


def load_text_chart(inputs: Inputs) -> Chart | None:
    if inputs.version != "jp":
        return None
    return Chart(cload.load_blob(cload.Inputs(inputs.version, inputs.config, inputs.assets)))


def extract(inputs: Inputs, output: Path) -> None:
    if output.exists():
        raise FileExistsError(f"{output} already exists")
    blob, groups = load_inputs(inputs)
    chart = load_text_chart(inputs)
    parts, group_parts = read_blob(blob, chart), read_groups(groups)

    def byte_map(source: Blob, ranges: list[Part]) -> dict:
        return {"source": source.file_name, "address": hex_address(source.address), "size": f"0x{len(source.data):X}",
                "ranges": [byte_map_entry(source, part) for part in ranges]}

    mapping = byte_map(blob, parts)
    mapping["rodata"] = [byte_map(groups, group_parts)]
    if chart:
        mapping["text_decoder"] = {
            "source": chart.blob.file_name, "overlay": "CLOAD",
            "address": hex_address(chart.blob.address + chart.start), "size": f"0x{chart.end - chart.start:X}",
            "note": "JP text uses CLOAD's character chart, outside GOSUB's data blob.",
        }
    output.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix=f".{output.name}-", dir=output.parent))
    try:
        for part in parts + group_parts:
            if part.content is not None:
                write_part(part.content, staging / part.file)
        dump_yaml(staging / "byte-map.yaml", mapping)
        staging.rename(output)
    except BaseException:
        shutil.rmtree(staging, ignore_errors=True)
        raise


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("output", type=Path, help="new folder to write")
    parser.add_argument("--version", choices=("us", "jp"), default="us")
    parser.add_argument("--config", type=Path, help="config folder (default config/<version>)")
    parser.add_argument("--assets", type=Path, help="splat assets (default assets/<version>)")
    args = parser.parse_args()
    inputs = Inputs(args.version, args.config or REPO_ROOT / "config" / args.version,
                    args.assets or REPO_ROOT / "assets" / args.version)
    try:
        extract(inputs, args.output)
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
