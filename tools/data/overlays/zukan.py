#!/usr/bin/env python3
"""Export everything ZUKAN, the encyclopedia screen, carries around.

That's the two textures, every entry and category name, the sprites for the
frame and buttons, and the tables that decide which entries each category
lists. The entry pages themselves aren't in here. They're separate CD
resources.

Most of it comes from the zukan_data blob that splat extracts after the code.
The tables are different. They belong to zukan_build_category_entries, which
keeps them in local arrays, so they're compiled into zukan_category.c and
splat never writes them out as a file. We read those straight from the
decompressed overlay instead.

One thing that looks wrong in the export but isn't: the border texture's
previews use the page texture's palettes. Both textures upload their palettes
to the same VRAM row, and the page texture goes second, so it wins. The
previews show what you actually see in the game.

Usage:

    python3 -m tools.data.overlays.zukan --version us assets/exports/us/overlays/zukan
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import shutil
import struct
import sys
import tempfile

from tools.data.formats.psx_tim import TimImage, parse_tim
from tools.data.overlays import cload, icon_set, png, splat_config, symbols, text_table
from tools.data.overlays.card_data import Chart
from tools.data.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[3]
OVERLAY_CONFIG = "overlays/ZUKAN.BIN.yaml"
SYMBOL_FILE = "symbols/zukan_symbol_addrs.txt"
SOURCE = REPO_ROOT / "src/overlays/zukan/zukan.c"
CATEGORY_SOURCE = REPO_ROOT / "src/overlays/zukan/zukan_category.c"

# The archive is a count, then one offset per section, in this order.
ARCHIVE_SECTIONS = ("page", "border", "entry_names", "category_names")
ARCHIVE_HEADER = struct.Struct("<5I")
TEXTURE_UPLOADS = {"page": (320, 0), "border": (832, 256)}
CLUT_UPLOAD = (0, 498)

# A sprite's first byte picks its texture: 0 is the page, 1 is the border.
SPRITE_TEXTURES = ("page", "border")
SPRITE_RECORD = struct.Struct("<IIHH")
SPRITE_COUNT = 21
SPRITE_SCREEN_X_OFFSET = 8

# An entry with page value N opens CD resource ENTRY_RESOURCE_BASE + N.
ENTRY_RESOURCE_BASE = 0xBFC
DISPLAY_ORDER_END = 0x400

# The screen doesn't use the stored boundaries for every category. Characters
# stops early, World History stops early, and Techniques also lists the ability
# entries that Characters gave up.
CATEGORY_END_OVERRIDES = {7: 454, 8: 576}
TECHNIQUE_CATEGORY = 11
TECHNIQUE_EXTRA_FIRST = 454
TECHNIQUE_EXTRA_COUNT = 26

# World History is split into groups, counted from its first entry, and each
# group has one unlock bit.
HISTORY_GROUP_BASE = 480
HISTORY_GROUP_COUNT = 6
HISTORY_GROUP_FIRST_BIT = 0x122

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
TABLE_SECTION = ("zukan_category", ".rodata")


@dataclass(frozen=True)
class ZukanSymbols:
    """Where each resource starts. The first three are in the data blob, the rest in the tables."""

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
                "update SYMBOL_NAMES in tools/data/overlays/zukan.py."
            )
        return cls(**{key: named[name] for key, name in SYMBOL_NAMES.items()})


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
    """One of the archive's TIMs, plus the palettes it really gets drawn with."""

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
    """One UI sprite record: two packed words and a screen position."""

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
        return {
            "index": self.index,
            "attributes": hex_address(self.attributes),
            "texture_word": hex_address(self.texture),
            "texture": SPRITE_TEXTURES[self.draw_mode],
            "u": self.u,
            "v": self.v,
            "width": self.width,
            "height": self.height,
            "palette": self.palette,
            "x": self.x,
            "y": self.y,
        }


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


def table_lengths(blob: Blob[ZukanSymbols]) -> dict[str, int]:
    """How many values each table has.

    Nothing stores the lengths. The tables sit back to back, so each one just
    runs up to where the next one starts. The display order is last, and it
    ends with a 0x400 terminator.
    """
    names = blob.symbols
    raw = blob.data[blob.offset(names.display_order) :]
    order = [value for (value,) in struct.iter_unpack("<H", raw[: len(raw) // 2 * 2])]
    if DISPLAY_ORDER_END not in order:
        raise ValueError(f"display order has no 0x{DISPLAY_ORDER_END:X} terminator")

    return {
        "category_ranges": (names.group_ranges - names.category_ranges) // 4,
        "group_ranges": (names.entry_values - names.group_ranges) // 4,
        "entry_values": (names.display_order - names.entry_values) // 2,
        "display_order": order.index(DISPLAY_ORDER_END) + 1,
    }


# ---------------------------------------------------------------------------
# The data blob


def read_header(blob: Blob[ZukanSymbols]) -> tuple[Part, tuple[int, ...]]:
    """The archive header, and where each section starts and ends."""
    address = blob.symbols.archive
    raw = read_bytes(blob, address, address + ARCHIVE_HEADER.size, "archive header")
    count, *offsets = ARCHIVE_HEADER.unpack(raw)
    size = blob.symbols.sprites - address

    if count != len(ARCHIVE_SECTIONS) or offsets[0] != ARCHIVE_HEADER.size:
        raise ValueError("resource archive must start with four sections")
    if offsets != sorted(set(offsets)) or offsets[-1] >= size:
        raise ValueError("invalid resource archive section offsets")

    sections = []
    for index, (name, offset) in enumerate(zip(ARCHIVE_SECTIONS, offsets)):
        sections.append({"index": index, "name": name, "offset": f"0x{offset:X}",
                         "address": hex_address(address + offset)})

    values = {"section_count": count, "sections": sections, "note": "Offsets count from the start of the archive."}
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

    # The page texture's palettes get uploaded over the border's, so both
    # textures end up drawn with them.
    palettes = tuple(struct.iter_unpack("<16H", tims["page"].clut.payload))

    parts = {}
    for index, name in enumerate(ARCHIVE_SECTIONS[:2]):
        start = blob.offset(address + bounds[index])
        end = start + len(tims[name].to_bytes())
        texture = Texture(address + bounds[index], name, tims[name], palettes)
        parts[name] = Part(f"{name} TIM", start, end, f"images/{name}/image.yaml", texture)
    return parts


def read_text(blob: Blob[ZukanSymbols], bounds: tuple[int, ...], index: int, chart: Chart | None) -> Part:
    name = ARCHIVE_SECTIONS[index]
    label = name.replace("_", " ")
    address = blob.symbols.archive + bounds[index]
    raw = read_bytes(blob, address, blob.symbols.archive + bounds[index + 1], label)
    if blob.version == "jp" and chart is None:
        raise ValueError("JP ZUKAN text needs CLOAD's character chart")

    table = text_table.parse(raw, 0, text_table.TWO_BYTE_CODES[blob.version])
    entries = []
    for entry in table.entries:
        text = chart.decode(entry.data) if chart else entry.text
        entries.append({"index": entry.index, "offset": f"0x{entry.offset:X}", "text": text,
                        "bytes": entry.data.hex(" ")})

    start = blob.offset(address)
    section = TextSection(address, name, raw[: table.size], tuple(entries))
    return Part(label, start, start + table.size, f"text/{name}.yaml", section)


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
        texture_width = tim.pixels.width_words * 4
        fits = sprite.u + sprite.width <= texture_width and sprite.v + sprite.height <= tim.pixels.height
        if not sprite.width or not sprite.height or not fits:
            raise ValueError(f"UI sprite {index} lies outside its texture")
        if sprite.palette >= palette_count:
            raise ValueError(f"UI sprite {index} refers to an unknown palette")
        sprites.append(sprite)

    start = blob.offset(address)
    content = Sprites(address, raw, tuple(sprites), textures)
    return Part("UI sprites", start, start + len(raw), "images/sprites/sprites.yaml", content)


def read_variables(blob: Blob[ZukanSymbols]) -> Part:
    """The screen's variables at the end of the blob. They're all zero on disc, so normally there's nothing to save."""
    start = blob.offset(blob.symbols.variables)
    if any(blob.data[start:]):
        return Part("screen state", start, len(blob.data), "unknown/variables.bin", blob.data[start:])
    return Part("screen state", start, len(blob.data),
                note="Runtime variables and the entry list; all zero on disc, not exported.")


def read_blob(blob: Blob[ZukanSymbols], chart: Chart | None = None) -> tuple[list[Part], TextSection, TextSection]:
    """Everything in the data blob. The names come back separately because the tables use them."""
    header, bounds = read_header(blob)
    textures = read_textures(blob, bounds)
    names = read_text(blob, bounds, 2, chart)
    categories = read_text(blob, bounds, 3, chart)
    sprites = read_sprites(blob, {name: part.content for name, part in textures.items()})

    parts = [header, textures["page"], textures["border"], names, categories, sprites, read_variables(blob)]
    return cover_gaps(blob, parts), names.content, categories.content


# ---------------------------------------------------------------------------
# The entry tables


def entry_name(names: TextSection, index: int) -> str | None:
    if 0 <= index < len(names.entries):
        return names.entries[index]["text"]
    return None


def category_of(index: int, ranges: tuple[int, ...]) -> int | None:
    for category, (first, end) in enumerate(zip(ranges, ranges[1:])):
        if first <= index < end:
            return category
    return None


def table_part(blob: Blob, name: str, address: int, raw: bytes, values: dict[str, object]) -> Part:
    start = blob.offset(address)
    return Part(name.replace("_", " "), start, start + len(raw), f"tables/{name}.yaml", Table(address, values, raw))


def read_categories(blob: Blob[ZukanSymbols], count: int, names: TextSection,
                    categories: TextSection) -> tuple[Part, tuple[int, ...]]:
    address = blob.symbols.category_ranges
    raw = read_bytes(blob, address, address + count * 4, "category ranges")
    ranges = struct.unpack(f"<{count}i", raw)
    if list(ranges) != sorted(ranges) or len(categories.entries) != count - 1:
        raise ValueError("category ranges must rise and match the category names")

    entries = []
    for index, (first, end) in enumerate(zip(ranges, ranges[1:])):
        entry = {
            "index": index,
            "name": categories.entries[index]["text"],
            "first_entry": first,
            "stored_end": end,
            "list_end": CATEGORY_END_OVERRIDES.get(index, end),
        }
        if index == TECHNIQUE_CATEGORY:
            entry["extra_entries"] = [TECHNIQUE_EXTRA_FIRST, TECHNIQUE_EXTRA_FIRST + TECHNIQUE_EXTRA_COUNT]
        entry["first_name"] = entry_name(names, first)
        entries.append(entry)

    values = {
        "record_format": f"<{count}i",
        "entries": entries,
        "note": "A category lists the entries from first_entry up to (not including) list_end, then any "
                "extra_entries. When list_end and stored_end differ, the screen ignores the stored one.",
    }
    return table_part(blob, "categories", address, raw, values), ranges


def read_history_groups(blob: Blob[ZukanSymbols], count: int, names: TextSection) -> Part:
    address = blob.symbols.group_ranges
    raw = read_bytes(blob, address, address + count * 4, "history group ranges")
    groups = struct.unpack(f"<{count}i", raw)
    if count != HISTORY_GROUP_COUNT + 1 or list(groups) != sorted(groups):
        raise ValueError("history group ranges must rise and bound six groups")

    entries = []
    for index, (first, end) in enumerate(zip(groups, groups[1:])):
        first_entry = HISTORY_GROUP_BASE + first
        entries.append({
            "index": index,
            "first_entry": first_entry,
            "end_entry": HISTORY_GROUP_BASE + end,
            "first_name": entry_name(names, first_entry),
            "unlock_bit": f"0x{HISTORY_GROUP_FIRST_BIT + index:X}",
        })

    values = {
        "record_format": f"<{count}i",
        "stored_offsets": list(groups),
        "entries": entries,
        "note": f"The stored offsets count from entry {HISTORY_GROUP_BASE}, the first World History entry. "
                "A group stays locked until its bit is set in the save's encyclopedia bits.",
    }
    return table_part(blob, "history_groups", address, raw, values)


def read_entries(blob: Blob[ZukanSymbols], count: int, ranges: tuple[int, ...], names: TextSection) -> Part:
    address = blob.symbols.entry_values
    raw = read_bytes(blob, address, address + count * 2, "entry resource values")

    entries = []
    for index, value in enumerate(struct.unpack(f"<{count}h", raw)):
        entry = {"index": index, "name": entry_name(names, index), "category": category_of(index, ranges),
                 "value": value}
        if value:
            entry["cd_resource"] = f"0x{ENTRY_RESOURCE_BASE + value:X}"
        entries.append(entry)

    values = {
        "record_format": f"<{count}h",
        "entries": entries,
        "note": f"The entry's page is CD resource 0x{ENTRY_RESOURCE_BASE:X} + value, and 0 means it has no page. "
                "category goes by the stored boundaries, not the screen's overrides.",
    }
    return table_part(blob, "entries", address, raw, values)


def read_display_order(blob: Blob[ZukanSymbols], count: int, names: TextSection) -> Part:
    address = blob.symbols.display_order
    raw = read_bytes(blob, address, address + count * 2, "display order")
    order = struct.unpack(f"<{count}H", raw)
    if DISPLAY_ORDER_END not in order:
        raise ValueError(f"display order has no 0x{DISPLAY_ORDER_END:X} terminator")

    entries = []
    for position, index in enumerate(order[: order.index(DISPLAY_ORDER_END)]):
        entries.append({"position": position, "entry": index, "name": entry_name(names, index)})

    values = {
        "record_format": f"<{count}H",
        "terminator": f"0x{DISPLAY_ORDER_END:X}",
        "entries": entries,
        "note": "Every list goes in this order. An entry that isn't here never shows up.",
    }
    return table_part(blob, "display_order", address, raw, values)


def read_tables(blob: Blob[ZukanSymbols], names: TextSection, categories: TextSection) -> list[Part]:
    lengths = table_lengths(blob)
    category_part, ranges = read_categories(blob, lengths["category_ranges"], names, categories)
    parts = [
        category_part,
        read_history_groups(blob, lengths["group_ranges"], names),
        read_entries(blob, lengths["entry_values"], ranges, names),
        read_display_order(blob, lengths["display_order"], names),
    ]
    return cover_gaps(blob, parts)


# ---------------------------------------------------------------------------
# Writing


def palette_colors(palette: tuple[int, ...]) -> list[tuple[int, int, int, int]]:
    return [icon_set.bgr555_to_rgba(value) for value in palette]


def pixel_indices(tim: TimImage) -> list[int]:
    """The 4-bit pixels, two to a byte with the low nibble first."""
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
        "address": hex_address(content.address),
        "file": "image.tim",
        "width": width,
        "height": height,
        "upload": {"pixels": [x, y], "clut": list(CLUT_UPLOAD)},
        "stored_layout": tim.metadata(),
        "previews": previews,
        "stored_palettes": stored,
        "note": "The previews use the page texture's palettes, since those get uploaded last and are what the "
                "game draws with. Empty palettes get no preview. Color 0 is transparent and the rest are opaque.",
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

        pixels = []
        for y in range(sprite.v, sprite.v + sprite.height):
            for x in range(sprite.u, sprite.u + sprite.width):
                pixels.append(colors[indices[y * width + x]])

        file = f"{sprite.index:02d}.png"
        png.write_rgba(path.parent / file, sprite.width, sprite.height, pixels)
        entries.append({**sprite.document(), "file": file})

    dump_yaml(path, {
        "address": hex_address(content.address),
        "record_format": SPRITE_RECORD.format,
        "entries": entries,
        "bytes": content.raw.hex(" "),
        "note": f"The screen draws each sprite at (x + {SPRITE_SCREEN_X_OFFSET}, y). Sprites 0-5 are the buttons "
                "under the page: 0 lights up when you go to the previous entry, 3 for the next one and 5 for going "
                "back to the list, and 0-3 are dimmed in the list view. The previews don't include that tinting.",
    })


# ---------------------------------------------------------------------------
# Putting it together


def check_order(names: ZukanSymbols, keys: tuple[str, ...], start: int, end: int, where: str) -> None:
    """Make sure each of these symbols falls between start and end, and comes after the one before it."""
    previous = start - 1
    for key in keys:
        address = getattr(names, key)
        if not start <= address < end:
            raise ValueError(f"{SYMBOL_NAMES[key]} is outside {where}")
        if address <= previous:
            raise ValueError(f"{SYMBOL_NAMES[key]} is out of resource order")
        previous = address


def load_tables(config: Path, version: str) -> Blob[ZukanSymbols]:
    """The four entry tables, read from the decompressed overlay since splat doesn't extract them."""
    names = ZukanSymbols.load(config / SYMBOL_FILE)
    overlay = config / OVERLAY_CONFIG
    start, end = splat_config.section(overlay, *TABLE_SECTION)
    check_order(names, TABLE_KEYS, start, end, f"{TABLE_SECTION[0]}'s {TABLE_SECTION[1]}")
    return Blob(splat_config.read_image(overlay, start, end), start, "ZUKAN.BIN", version, names)


def load_inputs(inputs: Inputs) -> tuple[Blob[ZukanSymbols], Blob[ZukanSymbols]]:
    """The data blob splat extracted, and the entry tables."""
    names = ZukanSymbols.load(inputs.config / SYMBOL_FILE)
    files = splat_config.data_files(inputs.config / OVERLAY_CONFIG, inputs.assets)
    source = splat_config.file_containing(files, names.archive, SYMBOL_NAMES["archive"])
    if not source.path.exists():
        raise ValueError(f"{source.path} is missing; run make splat first")

    raw = source.path.read_bytes()
    if len(raw) != source.end - source.start:
        raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
    check_order(names, DATA_KEYS, source.start, source.end, str(source.path))

    blob = Blob(raw, source.start, source.path.name, inputs.version, names)
    return blob, load_tables(inputs.config, inputs.version)


def load_text_chart(inputs: Inputs) -> Chart | None:
    """JP text is decoded with CLOAD's character chart. US text doesn't need one."""
    if inputs.version != "jp":
        return None
    return Chart(cload.load_blob(cload.Inputs(inputs.version, inputs.config, inputs.assets)))


def byte_map(source: Blob, parts: list[Part]) -> dict:
    return {
        "source": source.file_name,
        "address": hex_address(source.address),
        "size": f"0x{len(source.data):X}",
        "ranges": [byte_map_entry(source, part) for part in parts],
    }


def extract(inputs: Inputs, output: Path) -> None:
    """Read everything first, then write it all to a temporary folder and move that into place.

    That way a bad input or a failed write never leaves a half-finished export behind.
    """
    if output.exists():
        raise FileExistsError(f"{output} already exists")

    blob, tables = load_inputs(inputs)
    chart = load_text_chart(inputs)
    parts, names, categories = read_blob(blob, chart)
    table_parts = read_tables(tables, names, categories)

    mapping = byte_map(blob, parts)
    mapping["rodata"] = [byte_map(tables, table_parts)]
    if chart:
        mapping["text_decoder"] = {
            "source": chart.blob.file_name,
            "overlay": "CLOAD",
            "address": hex_address(chart.blob.address + chart.start),
            "size": f"0x{chart.end - chart.start:X}",
            "note": "JP text is decoded with CLOAD's character chart, which isn't part of ZUKAN.",
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
    config = args.config or REPO_ROOT / "config" / args.version
    assets = args.assets or REPO_ROOT / "assets" / args.version
    try:
        extract(Inputs(args.version, config, assets), args.output)
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
