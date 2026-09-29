#!/usr/bin/env python3
"""Export GNAME's name-entry graphics, text and tables.

The build links one unchanged data blob; BSS stays with gname.c. This tool
reads the blob using the existing asset parsers, then writes PNGs and YAML.
JP text uses CLOAD's character chart. US retains a Japanese kanji table too,
but its chart lacks those mappings, so those entries stay as glyph codes.

Like ADDHERO, readers return Parts, the byte map covers every byte, and a
finished export is moved into place only after all its files are written.

Example:

    python3 -m tools.overlays.gname --version us assets/exports/us/overlays/gname
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import shutil
import struct
import sys
import tempfile

from tools.assets.glyph_metrics import GlyphMetrics
from tools.assets.index_boundaries import IndexBoundaries
from tools.assets.index_map import IndexMap
from tools.assets.name_entry_resource import DICTIONARY_TOKENS, NameEntryResource, TABLE_NAMES
from tools.assets.psx_tim import TimImage, parse_embedded_tim
from tools.assets.sprite_animation import SpriteAnimation
from tools.assets.sprite_layout import SpriteLayout
from tools.assets.tab_cursor_layout import TabCursorLayout
from tools.overlays import cload, icon_set, png, splat_config, symbols, text_table
from tools.overlays.card_data import Chart
from tools.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[2]
OVERLAY_CONFIG = "overlays/GNAME.BIN.yaml"
SYMBOL_FILE = "symbols/gname_symbol_addrs.txt"
GLYPH_COUNT = 39
PANEL_BOUNDARY_COUNT = 5
KANJI_BOUNDARY_COUNT = 45
CURSOR_COUNT = 13
LAYOUT_SPRITE_COUNT = 20
ANIMATION_FRAME_COUNT = 7
ANIMATION_SLOT_COUNT = 3
UNMAPPED_CATEGORY = 0xFF


# ---------------------------------------------------------------------------
# Inputs and decoded parts


@dataclass(frozen=True)
class GnameSymbols:
    """Resource boundaries, in the order they appear in the data blob."""

    panels: int
    categories: int
    glyphs: int
    cursors: int
    kanji_offsets: int
    records: int
    texture: int
    layout: int
    animation: int
    variables: int

    @classmethod
    def load(cls, path: Path) -> GnameSymbols:
        named = symbols.load(path)
        missing = [name for name in SYMBOL_NAMES.values() if name not in named]
        if missing:
            raise ValueError(
                f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES in tools/overlays/gname.py."
            )
        return cls(**{key: named[name] for key, name in SYMBOL_NAMES.items()})


SYMBOL_NAMES = {
    "panels": "g_panel_char_offsets",
    "categories": "g_kanji_cat_entries",
    "glyphs": "g_glyph_table",
    "cursors": "g_tab_cursor_pos",
    "kanji_offsets": "g_kanji_entry_offsets",
    "records": "g_panel_data_base",
    "texture": "g_name_entry_tim",
    "layout": "g_layout_sprite_sequence",
    "animation": "g_glyph_append_anim_frames",
    "variables": "g_custom_name_buf",
}


@dataclass(frozen=True)
class Inputs:
    version: str
    config: Path
    assets: Path


@dataclass(frozen=True)
class Table:
    address: int
    document: dict[str, object]
    raw: bytes


@dataclass(frozen=True)
class Names:
    """The four decoded record tables and the complete original archive."""

    address: int
    raw: bytes
    header: dict[str, object]
    tables: tuple[dict[str, object], ...]


@dataclass(frozen=True)
class Texture:
    """The original TIM, with the metrics used to crop its UI sprites."""

    address: int
    image: TimImage
    glyphs: GlyphMetrics


def read_bytes(blob: Blob, address: int, end: int, what: str) -> bytes:
    start, stop = blob.offset(address), blob.offset(end)
    if not 0 <= start <= stop <= len(blob.data):
        raise ValueError(f"{what} is outside {blob.file_name}")
    return blob.data[start:stop]


def table_part(blob: Blob, name: str, address: int, raw: bytes, document: dict) -> Part:
    start = blob.offset(address)
    return Part(name, start, start + len(raw), f"tables/{name}.yaml", Table(address, document, raw))


# ---------------------------------------------------------------------------
# Reading the blob, in address order


def read_tables(blob: Blob[GnameSymbols], glyphs: GlyphMetrics) -> list[Part]:
    names = blob.symbols
    parts = []
    for name, address, end, count in (
        ("panel_record_boundaries", names.panels, names.categories, PANEL_BOUNDARY_COUNT),
        ("kanji_entry_offsets", names.kanji_offsets, names.records, KANJI_BOUNDARY_COUNT),
    ):
        raw = read_bytes(blob, address, end, name)
        table = IndexBoundaries.parse_binary(raw, count)
        parts.append(table_part(blob, name, address, raw, table.document()))
    raw = read_bytes(blob, names.categories, names.glyphs, "kanji category map")
    categories = IndexMap.parse_binary(raw, UNMAPPED_CATEGORY)
    if any(value is not None and value >= KANJI_BOUNDARY_COUNT - 1 for value in categories.entries):
        raise ValueError("kanji category points outside the boundary table")
    parts.append(table_part(blob, "kanji_category_map", names.categories, raw, categories.document()))
    raw = read_bytes(blob, names.glyphs, names.cursors, "glyph metrics")
    parts.append(table_part(blob, "glyph_metrics", names.glyphs, raw, glyphs.document()))
    raw = read_bytes(blob, names.cursors, names.kanji_offsets, "tab cursors")
    cursors = TabCursorLayout.parse_binary(raw, CURSOR_COUNT)
    if any(entry.glyph_id >= len(glyphs.glyphs) for entry in cursors.entries):
        raise ValueError("tab cursor refers to an unknown glyph")
    parts.append(table_part(blob, "tab_cursor_layout", names.cursors, raw, cursors.document()))
    return parts


def decode_record(raw: bytes, version: str, table: str, chart: Chart | None) -> str:
    """Decode a display string without treating a glyph's zero second byte as its end."""
    if not raw:
        return ""
    # The unused US kanji table still has JP's seven double-byte lead codes.
    encoding = "jp" if table == "kanji_records" else version
    end = text_table.string_end(raw, 0, text_table.TWO_BYTE_CODES[encoding])
    if end < 0:
        raise ValueError(f"{table} record has no string terminator")
    body = raw[:end]
    if version == "jp":
        if chart is None:
            raise ValueError("JP GNAME text needs CLOAD's character chart")
        return chart.decode(body)
    if table != "panel_records":
        return text_table.decode(body, text_table.TWO_BYTE_CODES[encoding])
    out = []
    cursor = 0
    while cursor < len(body):
        size = 2 if body[cursor] in text_table.TWO_BYTE_CODES["us"] else 1
        code = body[cursor : cursor + size]
        out.append(DICTIONARY_TOKENS.get(code, text_table.decode(code, text_table.TWO_BYTE_CODES["us"])))
        cursor += size
    return "".join(out)


def read_names(blob: Blob[GnameSymbols], chart: Chart | None) -> Part:
    address = blob.symbols.records
    raw = read_bytes(blob, address, blob.symbols.texture, "name records")
    resource = NameEntryResource.parse_binary(raw)
    unknown, *offsets = struct.unpack_from("<5I", raw)
    tables = []
    for name, offset, table in zip(TABLE_NAMES, offsets, resource.tables):
        entries = []
        position = len(table.records) * 2
        for index, record in enumerate(table.records):
            entries.append({
                "index": index, "offset": f"0x{position:X}",
                "text": decode_record(record, blob.version, name, chart),
                "bytes": record.hex(" "),
            })
            position += len(record)
        tables.append({"name": name, "address": hex_address(address + offset), "entries": entries})
    # These are record indices, including the final exclusive boundary.
    for key, table_index, count in (("panels", 0, PANEL_BOUNDARY_COUNT), ("kanji_offsets", 1, KANJI_BOUNDARY_COUNT)):
        start = getattr(blob.symbols, key)
        boundaries = struct.unpack("<" + "I" * count, read_bytes(blob, start, start + count * 4, key))
        if boundaries[-1] != len(resource.tables[table_index].records):
            raise ValueError(f"{key} final boundary differs from its record count")
    header = {
        "unknown_0x00": unknown,
        "tables": [{"name": name, "offset": f"0x{offset:X}", "file": f"{name}.yaml"}
                   for name, offset in zip(TABLE_NAMES, offsets)],
    }
    content = Names(address, raw, header, tuple(tables))
    start = blob.offset(address)
    return Part("name records", start, start + len(raw), "text/resource.yaml", content)


def read_texture(blob: Blob[GnameSymbols], glyphs: GlyphMetrics) -> list[Part]:
    address = blob.symbols.texture
    raw = read_bytes(blob, address, blob.symbols.layout, "name-entry TIM")
    tim = parse_embedded_tim(raw, trailing_duplicate_word=True)
    if tim.pixel_mode != 0 or tim.clut is None or len(tim.clut.payload) % 32:
        raise ValueError("name-entry TIM must have 4-bit pixels and complete 16-color palettes")
    width, height = tim.pixels.width_words * 4, tim.pixels.height
    palette_count = len(tim.clut.payload) // 32
    for index, glyph in enumerate(glyphs.glyphs):
        if not glyph.width or not glyph.height or glyph.u + glyph.width > width or glyph.v + glyph.height > height:
            raise ValueError(f"glyph {index} lies outside the name-entry TIM")
        if glyph.clut >= palette_count:
            raise ValueError(f"glyph {index} refers to an unknown palette")
    start = blob.offset(address)
    end = start + len(tim.to_bytes())
    parts = [Part("name-entry TIM", start, end, "image/image.yaml", Texture(address, tim, glyphs))]
    if end < blob.offset(blob.symbols.layout):
        parts.append(Part("TIM trailing word", end, end + 4, note="Repeats the TIM's last word."))
    return parts


def read_layout(blob: Blob[GnameSymbols]) -> list[Part]:
    names = blob.symbols
    raw = read_bytes(blob, names.layout, names.animation, "sprite layout")
    layout = SpriteLayout.parse_binary(raw, LAYOUT_SPRITE_COUNT)
    if any(sprite.glyph_id >= GLYPH_COUNT for sprite in layout.sprites):
        raise ValueError("layout refers to an unknown glyph")
    parts = [table_part(blob, "layout_sprite_sequence", names.layout, raw, layout.document())]
    size = ANIMATION_FRAME_COUNT * ANIMATION_SLOT_COUNT * 4
    raw = read_bytes(blob, names.animation, names.animation + size, "glyph append animation")
    animation = SpriteAnimation.parse_binary(raw, ANIMATION_FRAME_COUNT, ANIMATION_SLOT_COUNT, 0)
    if any(sprite.glyph_id >= GLYPH_COUNT for frame in animation.frames for sprite in frame.sprites):
        raise ValueError("append animation refers to an unknown glyph")
    document = animation.document()
    document["note"] = "Glyph zero hides a slot. Only the first slot's control byte gives the frame duration in render ticks."
    parts.append(table_part(blob, "glyph_append_animation", names.animation, raw, document))
    return parts


def read_blob(blob: Blob[GnameSymbols], chart: Chart | None = None) -> list[Part]:
    raw = read_bytes(blob, blob.symbols.glyphs, blob.symbols.cursors, "glyph metrics")
    glyphs = GlyphMetrics.parse_binary(raw, GLYPH_COUNT)
    parts = read_tables(blob, glyphs)
    parts.append(read_names(blob, chart))
    parts += read_texture(blob, glyphs)
    parts += read_layout(blob)
    return cover_gaps(blob, sorted(parts, key=lambda part: part.start))


# ---------------------------------------------------------------------------
# Writing


@write_part.register
def _write_table(content: Table, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), **content.document, "bytes": content.raw.hex(" ")})


@write_part.register
def _write_names(content: Names, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), "file": "resource.bin", **content.header})
    (path.parent / "resource.bin").write_bytes(content.raw)
    for table in content.tables:
        dump_yaml(path.parent / f"{table['name']}.yaml", table)


@write_part.register
def _write_texture(content: Texture, path: Path) -> None:
    tim = content.image
    width, height = tim.pixels.width_words * 4, tim.pixels.height
    palettes = tuple(struct.iter_unpack("<16H", tim.clut.payload))
    indices = [nibble for byte in tim.pixels.payload for nibble in (byte & 15, byte >> 4)]
    previews = []
    path.parent.mkdir(parents=True, exist_ok=True)
    (path.parent / "image.tim").write_bytes(tim.to_bytes())
    for index, palette in enumerate(palettes):
        colors = [icon_set.bgr555_to_rgba(value) for value in palette]
        file = f"palette_{index:02X}.png"
        png.write_rgba(path.parent / file, width, height, [colors[value] for value in indices])
        previews.append({"index": index, "file": file, "colors": [f"0x{value:04X}" for value in palette]})
    glyphs = []
    (path.parent / "glyphs").mkdir()
    for index, glyph in enumerate(content.glyphs.glyphs):
        colors = [icon_set.bgr555_to_rgba(value) for value in palettes[glyph.clut]]
        pixels = [colors[indices[y * width + x]]
                  for y in range(glyph.v, glyph.v + glyph.height)
                  for x in range(glyph.u, glyph.u + glyph.width)]
        file = f"glyphs/{index:02d}.png"
        png.write_rgba(path.parent / file, glyph.width, glyph.height, pixels)
        glyphs.append({"index": index, "file": file, **glyph.document()})
    dump_yaml(path, {
        "address": hex_address(content.address), "file": "image.tim",
        "width": width, "height": height, "stored_layout": tim.metadata(),
        "palettes": previews, "glyphs": glyphs,
        "note": "Palette value zero is transparent; other colors are shown opaque. "
                "GNAME uploads pixels at (320, 0) and its 256 colors in one row at (0, 498), "
                "setting STP on nonzero colors. The TIM here keeps the original words and coordinates.",
    })


# ---------------------------------------------------------------------------
# Putting it together


def load_blob(inputs: Inputs) -> Blob[GnameSymbols]:
    names = GnameSymbols.load(inputs.config / SYMBOL_FILE)
    files = splat_config.data_files(inputs.config / OVERLAY_CONFIG, inputs.assets)
    source = splat_config.file_containing(files, names.panels, SYMBOL_NAMES["panels"])
    if not source.path.exists():
        raise ValueError(f"{source.path} is missing; run make splat first")
    data = source.path.read_bytes()
    if len(data) != source.end - source.start:
        raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
    previous = source.start - 1
    for key in SYMBOL_NAMES:
        address = getattr(names, key)
        if key == "variables":
            if address != source.end:
                raise ValueError("GNAME data must end where g_custom_name_buf starts")
        elif not source.contains(address):
            raise ValueError(f"{SYMBOL_NAMES[key]} is outside {source.path}")
        if address <= previous:
            raise ValueError(f"{SYMBOL_NAMES[key]} is out of resource order")
        previous = address
    return Blob(data, source.start, source.path.name, inputs.version, names)


def load_text_chart(inputs: Inputs) -> Chart | None:
    if inputs.version != "jp":
        return None
    return Chart(cload.load_blob(cload.Inputs(inputs.version, inputs.config, inputs.assets)))


def extract(inputs: Inputs, output: Path) -> None:
    """Read first, write into a new staging folder, then move it into place."""
    if output.exists():
        raise FileExistsError(f"{output} already exists")
    blob = load_blob(inputs)
    chart = load_text_chart(inputs)
    parts = read_blob(blob, chart)
    byte_map = {
        "source": blob.file_name, "address": hex_address(blob.address),
        "size": f"0x{len(blob.data):X}", "ranges": [byte_map_entry(blob, part) for part in parts],
    }
    if chart:
        byte_map["text_decoder"] = {
            "source": chart.blob.file_name, "overlay": "CLOAD",
            "address": hex_address(chart.blob.address + chart.start),
            "size": f"0x{chart.end - chart.start:X}",
            "note": "JP text uses CLOAD's character chart. GNAME does not contain this conversion table.",
        }
    output.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix=f".{output.name}-", dir=output.parent))
    try:
        for part in parts:
            if part.content is not None:
                write_part(part.content, staging / part.file)
        dump_yaml(staging / "byte-map.yaml", byte_map)
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
