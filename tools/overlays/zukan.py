#!/usr/bin/env python3
"""Export ZUKAN's encyclopedia artwork, entry names and category tables.

The build links one data blob after the code and one rodata blob of entry
tables before it. Readers return Parts; the byte maps cover both inputs. The
overlay's entry function, zukan_run, sits between the tables and the C code in
a rodatabin of its own; it is code, so it is not an input here.

The data blob starts with a four-section archive: the page and border
textures, the entry names and the category names. Both textures upload their
CLUT to the same VRAM row, and the page texture goes last, so every sprite is
drawn through the page texture's palettes. Previews use those runtime colors.

Example:

    python3 -m tools.overlays.zukan --version us assets/exports/us/overlays/zukan
"""

from __future__ import annotations

import argparse
import ast
from dataclasses import dataclass
from pathlib import Path
import re
import shutil
import struct
import sys
import tempfile

from tools.assets.psx_tim import TimImage, parse_tim
from tools.overlays import cload, icon_set, png, splat_config, symbols, text_table
from tools.overlays.card_data import Chart
from tools.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[2]
OVERLAY_CONFIG = "overlays/ZUKAN.BIN.yaml"
SYMBOL_FILE = "symbols/zukan_symbol_addrs.txt"
SOURCE = REPO_ROOT / "src/overlays/zukan/zukan.c"
CATEGORY_SOURCE = REPO_ROOT / "src/overlays/zukan/zukan_category.c"

# Archive sections, in the order of the offset words that follow the count.
ARCHIVE_SECTIONS = ("page", "border", "entry_names", "category_names")
ARCHIVE_HEADER = struct.Struct("<5I")
TEXTURE_UPLOADS = {"page": (320, 0), "border": (832, 256)}
CLUT_UPLOAD = (0, 498)
# The draw-mode byte of a UI sprite selects the texture page: 0 -> page, 1 -> border.
SPRITE_TEXTURES = ("page", "border")
SPRITE_RECORD = struct.Struct("<IIHH")
SPRITE_COUNT = 21
SPRITE_SCREEN_X_OFFSET = 8
ENTRY_RESOURCE_BASE = 0xBFC
DISPLAY_ORDER_END = 0x400
# zukan_build_category_entries shortens two categories and extends another.
CATEGORY_END_OVERRIDES = {7: 0x1C6, 8: 0x240}
TECHNIQUE_CATEGORY = 11
TECHNIQUE_EXTRA_FIRST = 0x1C6
TECHNIQUE_EXTRA_COUNT = 0x1A
HISTORY_GROUP_BASE = 0x1E0
HISTORY_GROUP_COUNT = 6
HISTORY_GROUP_FIRST_BIT = 0x122


# ---------------------------------------------------------------------------
# Inputs and decoded parts


@dataclass(frozen=True)
class ZukanSymbols:
    """Resource boundaries: three in the data blob, four in the table rodata."""

    archive: int
    sprites: int
    variables: int
    category_ranges: int
    group_ranges: int
    entry_values: int
    display_order: int

    @classmethod
    def load(cls, path: Path) -> ZukanSymbols:
        named = symbols.load(path)
        missing = [name for name in SYMBOL_NAMES.values() if name not in named]
        if missing:
            raise ValueError(
                f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES in tools/overlays/zukan.py."
            )
        return cls(**{key: named[name] for key, name in SYMBOL_NAMES.items()})


SYMBOL_NAMES = {
    "archive": "g_zukan_resource_archive",
    "sprites": "g_zukan_ui_sprites",
    "variables": "g_zukan_resource_buffer",
    "category_ranges": "g_zukan_category_ranges",
    "group_ranges": "g_zukan_group_ranges",
    "entry_values": "g_zukan_entry_values",
    "display_order": "g_zukan_display_order",
}
DATA_KEYS = ("archive", "sprites", "variables")
TABLE_KEYS = ("category_ranges", "group_ranges", "entry_values", "display_order")


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
class Texture:
    """One archive TIM and the runtime palettes it is drawn with."""

    address: int
    name: str
    tim: TimImage
    palettes: tuple[tuple[int, ...], ...]


@dataclass(frozen=True)
class TextSection:
    address: int
    name: str
    raw: bytes
    entries: tuple[dict[str, object], ...]


@dataclass(frozen=True)
class Sprite:
    index: int
    attributes: int
    texture: int
    x: int
    y: int

    @property
    def draw_mode(self) -> int:
        return self.attributes & 0xFF

    @property
    def u(self) -> int:
        return (self.attributes >> 8) & 0xFF

    @property
    def v(self) -> int:
        return self.texture & 0xFF

    @property
    def palette(self) -> int:
        return (self.texture >> 8) & 0x3F

    @property
    def width(self) -> int:
        return (self.texture >> 14) & 0x1FF

    @property
    def height(self) -> int:
        return self.texture >> 23

    def document(self) -> dict[str, object]:
        return {"index": self.index, "attributes": hex_address(self.attributes), "texture_word": hex_address(self.texture),
                "texture": SPRITE_TEXTURES[self.draw_mode], "u": self.u, "v": self.v, "width": self.width,
                "height": self.height, "palette": self.palette, "x": self.x, "y": self.y}


@dataclass(frozen=True)
class Sprites:
    address: int
    raw: bytes
    sprites: tuple[Sprite, ...]
    textures: dict[str, Texture]


def read_bytes(blob: Blob, address: int, end: int, what: str) -> bytes:
    start, stop = blob.offset(address), blob.offset(end)
    if not 0 <= start <= stop <= len(blob.data):
        raise ValueError(f"{what} is outside {blob.file_name}")
    return blob.data[start:stop]


def table_lengths() -> dict[str, int]:
    """Array lengths of the four table structs copied by zukan_build_category_entries."""
    source = CATEGORY_SOURCE.read_text(encoding="ascii")
    defines = dict(re.findall(r"^#define\s+(\w+)\s+([^\n]+)", source, re.MULTILINE))

    def array_length(expression: str, seen: frozenset[str] = frozenset()) -> int:
        """Resolve integer literals, named counts and sums used by the table arrays."""
        node = ast.parse(expression, mode="eval").body
        if isinstance(node, ast.Constant) and type(node.value) is int:
            return node.value
        if isinstance(node, ast.Name) and node.id in defines and node.id not in seen:
            return array_length(defines[node.id], seen | {node.id})
        if isinstance(node, ast.BinOp) and isinstance(node.op, ast.Add):
            return array_length(ast.unparse(node.left), seen) + array_length(ast.unparse(node.right), seen)
        raise ValueError(f"{CATEGORY_SOURCE} has an unsupported table length: {expression}")

    lengths = {}
    for key, type_name in (("category_ranges", "ZukanCategoryRangeTable"), ("group_ranges", "ZukanGroupRangeTable"),
                           ("entry_values", "ZukanEntryValueTable"), ("display_order", "ZukanDisplayOrderTable")):
        match = re.search(r"typedef struct\s*\{\s*[su](16|32) values\[([^\]]+)\];\s*\}\s*" + type_name + ";", source)
        if match is None:
            raise ValueError(f"{CATEGORY_SOURCE} has no {type_name}")
        lengths[key] = array_length(match[2])
    return lengths


# ---------------------------------------------------------------------------
# Reading the data blob


def read_header(blob: Blob[ZukanSymbols]) -> tuple[Part, tuple[int, ...]]:
    address = blob.symbols.archive
    raw = read_bytes(blob, address, address + ARCHIVE_HEADER.size, "archive header")
    count, *offsets = ARCHIVE_HEADER.unpack(raw)
    size = blob.symbols.sprites - address
    if count != len(ARCHIVE_SECTIONS) or offsets[0] != ARCHIVE_HEADER.size:
        raise ValueError("resource archive must start with four sections")
    if offsets != sorted(set(offsets)) or offsets[-1] >= size:
        raise ValueError("invalid resource archive section offsets")
    sections = [{"index": index, "name": name, "offset": f"0x{offset:X}", "address": hex_address(address + offset)}
                for index, (name, offset) in enumerate(zip(ARCHIVE_SECTIONS, offsets))]
    values = {"section_count": count, "sections": sections,
              "note": "Offsets count from the start of the archive."}
    start = blob.offset(address)
    part = Part("archive header", start, start + len(raw), "archive/archive.yaml", Table(address, values, raw))
    return part, tuple(offsets) + (size,)


def read_textures(blob: Blob[ZukanSymbols], bounds: tuple[int, ...]) -> dict[str, Part]:
    address = blob.symbols.archive
    tims = {}
    for index, name in enumerate(ARCHIVE_SECTIONS[:2]):
        raw = read_bytes(blob, address + bounds[index], address + bounds[index + 1], f"{name} TIM")
        tim = parse_tim(raw)
        if tim.pixel_mode != 0 or tim.clut is None or not tim.clut.payload or len(tim.clut.payload) % 32:
            raise ValueError(f"{name} TIM must have 4-bit pixels and complete 16-color palettes")
        tims[name] = tim
    # The page texture's CLUT is uploaded last, over the border's, so both draw through it.
    palettes = tuple(struct.iter_unpack("<16H", tims["page"].clut.payload))
    parts = {}
    for index, name in enumerate(ARCHIVE_SECTIONS[:2]):
        start = blob.offset(address + bounds[index])
        content = Texture(address + bounds[index], name, tims[name], palettes)
        parts[name] = Part(f"{name} TIM", start, start + len(tims[name].to_bytes()), f"images/{name}/image.yaml", content)
    return parts


def read_text(blob: Blob[ZukanSymbols], bounds: tuple[int, ...], index: int, chart: Chart | None) -> Part:
    name = ARCHIVE_SECTIONS[index]
    address = blob.symbols.archive + bounds[index]
    raw = read_bytes(blob, address, blob.symbols.archive + bounds[index + 1], name.replace("_", " "))
    if blob.version == "jp" and chart is None:
        raise ValueError("JP ZUKAN text needs CLOAD's character chart")
    table = text_table.parse(raw, 0, text_table.TWO_BYTE_CODES[blob.version])
    entries = tuple({"index": entry.index, "offset": f"0x{entry.offset:X}",
                     "text": chart.decode(entry.data) if chart else entry.text,
                     "bytes": entry.data.hex(" ")} for entry in table.entries)
    start = blob.offset(address)
    return Part(name.replace("_", " "), start, start + table.size, f"text/{name}.yaml",
                TextSection(address, name, raw[:table.size], entries))


def read_sprites(blob: Blob[ZukanSymbols], textures: dict[str, Texture]) -> Part:
    address = blob.symbols.sprites
    raw = read_bytes(blob, address, address + SPRITE_COUNT * SPRITE_RECORD.size, "UI sprites")
    if blob.offset(address) + len(raw) > blob.offset(blob.symbols.variables):
        raise ValueError("UI sprites run into the runtime variables")
    palette_count = len(textures["page"].palettes)
    sprites = []
    for index, values in enumerate(SPRITE_RECORD.iter_unpack(raw)):
        sprite = Sprite(index, *values)
        if sprite.draw_mode >= len(SPRITE_TEXTURES):
            raise ValueError(f"UI sprite {index} has an unknown draw mode {sprite.draw_mode}")
        tim = textures[SPRITE_TEXTURES[sprite.draw_mode]].tim
        if not sprite.width or not sprite.height or sprite.u + sprite.width > tim.pixels.width_words * 4 \
                or sprite.v + sprite.height > tim.pixels.height:
            raise ValueError(f"UI sprite {index} lies outside its texture")
        if sprite.palette >= palette_count:
            raise ValueError(f"UI sprite {index} refers to an unknown palette")
        sprites.append(sprite)
    start = blob.offset(address)
    return Part("UI sprites", start, start + len(raw), "images/sprites/sprites.yaml",
                Sprites(address, raw, tuple(sprites), textures))


def read_variables(blob: Blob[ZukanSymbols]) -> Part:
    start = blob.offset(blob.symbols.variables)
    if any(blob.data[start:]):
        return Part("screen state", start, len(blob.data), "unknown/variables.bin", blob.data[start:])
    return Part("screen state", start, len(blob.data), note="Runtime variables and the entry list; all zero on disc, not exported.")


def read_blob(blob: Blob[ZukanSymbols], chart: Chart | None = None) -> tuple[list[Part], TextSection, TextSection]:
    """Parts of the data blob, plus the two text sections the table readers use for names."""
    header, bounds = read_header(blob)
    textures = read_textures(blob, bounds)
    names = read_text(blob, bounds, 2, chart)
    categories = read_text(blob, bounds, 3, chart)
    parts = [header, textures["page"], textures["border"], names, categories]
    parts.append(read_sprites(blob, {key: part.content for key, part in textures.items()}))
    parts.append(read_variables(blob))
    return cover_gaps(blob, parts), names.content, categories.content


# ---------------------------------------------------------------------------
# Reading the table rodata


def entry_name(names: TextSection, index: int) -> str | None:
    return names.entries[index]["text"] if 0 <= index < len(names.entries) else None


def table_part(blob: Blob, name: str, address: int, raw: bytes, values: dict[str, object]) -> Part:
    start = blob.offset(address)
    return Part(name.replace("_", " "), start, start + len(raw), f"tables/{name}.yaml", Table(address, values, raw))


def read_categories(blob: Blob[ZukanSymbols], count: int, names: TextSection, categories: TextSection) -> tuple[Part, tuple[int, ...]]:
    address = blob.symbols.category_ranges
    raw = read_bytes(blob, address, address + count * 4, "category ranges")
    ranges = struct.unpack(f"<{count}i", raw)
    if list(ranges) != sorted(ranges) or len(categories.entries) != count - 1:
        raise ValueError("category ranges must rise and match the category names")
    entries = []
    for index, (first, end) in enumerate(zip(ranges, ranges[1:])):
        shown_end = CATEGORY_END_OVERRIDES.get(index, end)
        entry = {"index": index, "name": categories.entries[index]["text"], "first_entry": first, "stored_end": end,
                 "list_end": shown_end}
        if index == TECHNIQUE_CATEGORY:
            entry["extra_entries"] = [TECHNIQUE_EXTRA_FIRST, TECHNIQUE_EXTRA_FIRST + TECHNIQUE_EXTRA_COUNT]
        entry["first_name"] = entry_name(names, first)
        entries.append(entry)
    values = {"record_format": f"<{count}i", "entries": entries,
              "note": "A category lists entry indices first_entry up to list_end (exclusive), then extra_entries. "
                      "list_end differs from stored_end where the screen overrides it."}
    return table_part(blob, "categories", address, raw, values), ranges


def read_history_groups(blob: Blob[ZukanSymbols], count: int, names: TextSection) -> Part:
    address = blob.symbols.group_ranges
    raw = read_bytes(blob, address, address + count * 4, "history group ranges")
    groups = struct.unpack(f"<{count}i", raw)
    if count != HISTORY_GROUP_COUNT + 1 or list(groups) != sorted(groups):
        raise ValueError("history group ranges must rise and bound six groups")
    base = HISTORY_GROUP_BASE
    entries = [{"index": index, "first_entry": base + first, "end_entry": base + end,
                "first_name": entry_name(names, base + first), "unlock_bit": f"0x{HISTORY_GROUP_FIRST_BIT + index:X}"}
               for index, (first, end) in enumerate(zip(groups, groups[1:]))]
    values = {"record_format": f"<{count}i", "stored_offsets": list(groups), "entries": entries,
              "note": f"Offsets count from entry {HISTORY_GROUP_BASE}, the first World History entry. "
                      "A group's entries are hidden until its bit in the saved game's encyclopedia bits is set."}
    return table_part(blob, "history_groups", address, raw, values)


def read_entries(blob: Blob[ZukanSymbols], count: int, ranges: tuple[int, ...], names: TextSection) -> Part:
    address = blob.symbols.entry_values
    raw = read_bytes(blob, address, address + count * 2, "entry resource values")
    entries = []
    for index, value in enumerate(struct.unpack(f"<{count}h", raw)):
        category = next((c for c, (first, end) in enumerate(zip(ranges, ranges[1:])) if first <= index < end), None)
        entry = {"index": index, "name": entry_name(names, index), "category": category, "value": value}
        if value:
            entry["cd_resource"] = f"0x{ENTRY_RESOURCE_BASE + value:X}"
        entries.append(entry)
    values = {"record_format": f"<{count}h", "entries": entries,
              "note": f"A nonzero value opens CD resource 0x{ENTRY_RESOURCE_BASE:X} + value as the entry's page. "
                      "Zero means the entry has no page. category follows the stored ranges."}
    return table_part(blob, "entries", address, raw, values)


def read_display_order(blob: Blob[ZukanSymbols], count: int, names: TextSection) -> Part:
    address = blob.symbols.display_order
    raw = read_bytes(blob, address, address + count * 2, "display order")
    order = struct.unpack(f"<{count}H", raw)
    if DISPLAY_ORDER_END not in order:
        raise ValueError(f"display order has no 0x{DISPLAY_ORDER_END:X} terminator")
    end = order.index(DISPLAY_ORDER_END)
    entries = [{"position": position, "entry": index, "name": entry_name(names, index)}
               for position, index in enumerate(order[:end])]
    values = {"record_format": f"<{count}H", "terminator": f"0x{DISPLAY_ORDER_END:X}", "entries": entries,
              "trailing_values": list(order[end + 1:]),
              "note": "Each category lists its entries in this order; entries missing here are not listed."}
    return table_part(blob, "display_order", address, raw, values)


def read_tables(blob: Blob[ZukanSymbols], names: TextSection, categories: TextSection) -> list[Part]:
    lengths = table_lengths()
    category_part, ranges = read_categories(blob, lengths["category_ranges"], names, categories)
    parts = [category_part,
             read_history_groups(blob, lengths["group_ranges"], names),
             read_entries(blob, lengths["entry_values"], ranges, names),
             read_display_order(blob, lengths["display_order"], names)]
    return cover_gaps(blob, parts)


# ---------------------------------------------------------------------------
# Writing


def palette_colors(palette: tuple[int, ...]) -> list[tuple[int, int, int, int]]:
    return [icon_set.bgr555_to_rgba(value) for value in palette]


def pixel_indices(tim: TimImage) -> list[int]:
    return [nibble for byte in tim.pixels.payload for nibble in (byte & 15, byte >> 4)]


@write_part.register
def _write_table(content: Table, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), **content.values, "bytes": content.raw.hex(" ")})


@write_part.register
def _write_texture(content: Texture, path: Path) -> None:
    tim = content.tim
    width, height = tim.pixels.width_words * 4, tim.pixels.height
    indices = pixel_indices(tim)
    path.parent.mkdir(parents=True, exist_ok=True)
    (path.parent / "image.tim").write_bytes(tim.to_bytes())
    previews = []
    for index, palette in enumerate(content.palettes):
        if not any(palette):
            continue
        colors = palette_colors(palette)
        file = f"palette_{index:02d}.png"
        png.write_rgba(path.parent / file, width, height, [colors[value] for value in indices])
        previews.append({"index": index, "file": file})
    stored = [[f"0x{value:04X}" for value in palette] for palette in struct.iter_unpack("<16H", tim.clut.payload)]
    x, y = TEXTURE_UPLOADS[content.name]
    dump_yaml(path, {
        "address": hex_address(content.address), "file": "image.tim", "width": width, "height": height,
        "upload": {"pixels": [x, y], "clut": list(CLUT_UPLOAD)},
        "stored_layout": tim.metadata(), "previews": previews, "stored_palettes": stored,
        "note": "Previews use the runtime palettes: the page TIM's CLUT, uploaded last to the shared row. "
                "Empty palettes have no preview. Zero is transparent; other colors are opaque.",
    })


@write_part.register
def _write_text(content: TextSection, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), "name": content.name, "entries": list(content.entries)})


@write_part.register
def _write_sprites(content: Sprites, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    entries = []
    for sprite in content.sprites:
        texture = content.textures[SPRITE_TEXTURES[sprite.draw_mode]]
        width = texture.tim.pixels.width_words * 4
        indices = pixel_indices(texture.tim)
        colors = palette_colors(texture.palettes[sprite.palette])
        pixels = [colors[indices[y * width + x]]
                  for y in range(sprite.v, sprite.v + sprite.height)
                  for x in range(sprite.u, sprite.u + sprite.width)]
        file = f"{sprite.index:02d}.png"
        png.write_rgba(path.parent / file, sprite.width, sprite.height, pixels)
        entries.append({**sprite.document(), "file": file})
    dump_yaml(path, {
        "address": hex_address(content.address), "record_format": SPRITE_RECORD.format, "entries": entries,
        "bytes": content.raw.hex(" "),
        "note": f"The screen draws each sprite at (x + {SPRITE_SCREEN_X_OFFSET}, y). Sprites 0-5 are the page controls: "
                "0 lights up for the previous entry, 3 for the next and 5 for the return to the list; "
                "0-3 are dimmed in the list view. Previews use the runtime palette without tinting.",
    })


# ---------------------------------------------------------------------------
# Putting it together


def load_inputs(inputs: Inputs) -> tuple[Blob[ZukanSymbols], Blob[ZukanSymbols]]:
    names = ZukanSymbols.load(inputs.config / SYMBOL_FILE)
    files = splat_config.data_files(inputs.config / OVERLAY_CONFIG, inputs.assets)

    def load(keys: tuple[str, ...]) -> Blob:
        source = splat_config.file_containing(files, getattr(names, keys[0]), SYMBOL_NAMES[keys[0]])
        if not source.path.exists():
            raise ValueError(f"{source.path} is missing; run make splat first")
        raw = source.path.read_bytes()
        if len(raw) != source.end - source.start:
            raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
        previous = source.start - 1
        for key in keys:
            address = getattr(names, key)
            if not source.contains(address):
                raise ValueError(f"{SYMBOL_NAMES[key]} is outside {source.path}")
            if address <= previous:
                raise ValueError(f"{SYMBOL_NAMES[key]} is out of resource order")
            previous = address
        return Blob(raw, source.start, source.path.name, inputs.version, names)

    return load(DATA_KEYS), load(TABLE_KEYS)


def load_text_chart(inputs: Inputs) -> Chart | None:
    if inputs.version != "jp":
        return None
    return Chart(cload.load_blob(cload.Inputs(inputs.version, inputs.config, inputs.assets)))


def extract(inputs: Inputs, output: Path) -> None:
    """Read everything, write a temporary folder, then move the finished export into place."""
    if output.exists():
        raise FileExistsError(f"{output} already exists")
    blob, tables = load_inputs(inputs)
    chart = load_text_chart(inputs)
    parts, names, categories = read_blob(blob, chart)
    table_parts = read_tables(tables, names, categories)

    def byte_map(source: Blob, ranges: list[Part]) -> dict:
        return {"source": source.file_name, "address": hex_address(source.address), "size": f"0x{len(source.data):X}",
                "ranges": [byte_map_entry(source, part) for part in ranges]}

    mapping = byte_map(blob, parts)
    mapping["rodata"] = [byte_map(tables, table_parts)]
    if chart:
        mapping["text_decoder"] = {
            "source": chart.blob.file_name, "overlay": "CLOAD",
            "address": hex_address(chart.blob.address + chart.start), "size": f"0x{chart.end - chart.start:X}",
            "note": "JP text uses CLOAD's character chart, outside ZUKAN's data blob.",
        }
    output.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix=f".{output.name}-", dir=output.parent))
    try:
        for part in parts + table_parts:
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
