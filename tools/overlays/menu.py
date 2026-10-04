#!/usr/bin/env python3
"""Export the ring menu's texture, icons, text, content layouts and tables.

The build links one unchanged data blob. Readers describe its resources as
Parts, and the byte map also records padding and the zero-filled runtime
variables. JP text uses CLOAD's character chart, as FIELD and GOLEM do.

Several symbols are index bases rather than object starts: the label-id
tables are read at fixed offsets from their symbols, and the input-script
table's unused row 0 holds the last twelve content-table pointers. The
readers follow what the C code reads, so each byte is exported once.

Example:

    python3 -m tools.overlays.menu --version us assets/exports/us/overlays/menu
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

from tools.assets.psx_tim import TimImage, parse_tim
from tools.overlays import cload, icon_set, png, splat_config, symbols, text_table
from tools.overlays.card_data import Chart
from tools.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[2]
OVERLAY_CONFIG = "overlays/MENU.BIN.yaml"
SYMBOL_FILE = "symbols/menu_symbol_addrs.txt"
HEADER = REPO_ROOT / "src/overlays/menu/internal/menu_internal.h"
PAD_HEADER = REPO_ROOT / "include/sdk/libetc.h"

# C layouts from menu_internal.h, menu.h and tim.h; tests keep them in step.
NODE_COUNT = 0x2C  # MENU_NODE_COUNT
GRID_SPRITE_COUNT = 0x1D  # MENU_GRID_SPRITE_COUNT
GRID_ALT_CLUT_START = 0x11  # MENU_GRID_ALT_CLUT_START
TIM_IMAGE_BLOCK_SIZE = 0x800C  # MENU_TIM_IMAGE_BLOCK_SIZE
CLUT_ENTRY_COUNT = 0x100  # CLUT_ENTRY_COUNT
ICON_CLUT_Y_BASE = 0x1F2  # MENU_ICON_CLUT_Y_BASE
SCRIPT_END = 0xFFFF  # MENU_SCRIPT_END
SCRIPT_INPUTS = 24  # MenuScript::inputs
CURSOR_FRAMES = 3  # g_menu_cursor_icon_ids
CONTENT_LOAD_REQUEST = 1  # g_menu_content_table value that sets g_menu_load_request
CONTENT_X_MASK = 0x1FF  # MENU_CONTENT_X_MASK
CONTENT_STYLE_SHIFT = 9  # MENU_CONTENT_STYLE_SHIFT
CONTENT_STYLE_MASK = 0x7  # MENU_CONTENT_STYLE_MASK
CONTENT_TYPE_SHIFT = 12  # MENU_CONTENT_TYPE_SHIFT
# menu_draw_scene_content reads D_80168659[action_type] for action types 0x37-0x54,
# the lowest index any code applies to the label-id bases.
ACTION_LABEL_FIRST = 0x37

ICON_RECORD = struct.Struct("<4B")  # MenuIconSpriteInfo
CONTENT_ITEM = struct.Struct("<HBB4s")  # MenuContentItem
GRID_SPRITE = struct.Struct("<HHhhhh")  # MenuGridSpriteDef: uv, pad, x, y, width, height
TIM_ASSET_HEADER = struct.Struct("<3I")  # MenuTimAsset: entry count and two offsets
SCRIPT_ROW = struct.Struct(f"<{SCRIPT_INPUTS}H")
ACTION_ROW = 8  # g_menu_content_action_codes[][8]
PALETTE_COLORS = 16
TEXTURE_SIZE = 256


# ---------------------------------------------------------------------------
# Inputs and decoded parts


@dataclass(frozen=True)
class MenuSymbols:
    """Resource anchors in stored order, plus the complete symbol map."""

    icon_sprites: int
    icon_palettes: int
    item_counts: int
    default_count: int
    action_codes: int
    default_items: int
    text: int
    image: int
    group_ids: int
    content_table: int
    scripts: int
    grid_sprites: int
    cursor_icons: int
    variables: int
    named: dict[str, int]

    @classmethod
    def load(cls, path: Path) -> MenuSymbols:
        named = symbols.load(path)
        missing = [name for name in (*SYMBOL_NAMES.values(), *LABEL_BASES) if name not in named]
        if missing:
            raise ValueError(
                f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES or LABEL_BASES in tools/overlays/menu.py."
            )
        return cls(**{key: named[name] for key, name in SYMBOL_NAMES.items()}, named=named)


SYMBOL_NAMES = {
    "icon_sprites": "g_menu_icon_sprite_defs",
    "icon_palettes": "g_menu_icon_clut_codes",
    "item_counts": "g_menu_content_item_counts",
    "default_count": "g_menu_default_content_count_minus_one",
    "action_codes": "g_menu_content_action_codes",
    "default_items": "g_menu_default_content_items",
    "text": "g_menu_state_data",
    "image": "g_menu_tim",
    "group_ids": "g_menu_content_group_ids",
    "content_table": "g_menu_content_table",
    "scripts": "g_script_table",
    "grid_sprites": "g_menu_glyph_src",
    "cursor_icons": "g_menu_cursor_icon_ids",
    "variables": "g_menu_display_delay",
}

# Index bases into the label-id bytes, in address order. Each is read as
# base[index] with index well above zero, so a base can sit before the bytes.
LABEL_BASES = ("D_80168659", "D_80168696", "D_8016869B", "D_8016869F", "D_801686A0", "D_801686B8")


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
    """The menu texture's 4-bit pixels and its palettes, keyed by palette code.

    Codes 0x00-0x0F are the TIM's own palettes, uploaded to VRAM row 498;
    codes 0x10-0x1F are the second palette row, uploaded to row 499.
    """

    indices: tuple[int, ...]
    palettes: dict[int, tuple[int, ...]]

    def colors(self, code: int) -> list[tuple[int, int, int, int]]:
        return [icon_set.bgr555_to_rgba(value) for value in self.palettes[code]]

    def crop(self, u: int, v: int, width: int, height: int, code: int) -> list[tuple[int, int, int, int]]:
        colors = self.colors(code)
        return [colors[self.indices[(v + y) * TEXTURE_SIZE + u + x]] for y in range(height) for x in range(width)]


@dataclass(frozen=True)
class MenuImage:
    address: int
    raw: bytes
    tim: TimImage
    texture: Texture
    header: dict[str, object]


@dataclass(frozen=True)
class Icons:
    address: int
    entries: tuple[dict[str, object], ...]
    texture: Texture
    raw: bytes


@dataclass(frozen=True)
class GridSprites:
    address: int
    entries: tuple[dict[str, object], ...]
    texture: Texture
    raw: bytes


@dataclass(frozen=True)
class TextResource:
    address: int
    raw: bytes
    tables: tuple[dict[str, object], ...]


def read_bytes(blob: Blob, address: int, end: int, what: str) -> bytes:
    start, stop = blob.offset(address), blob.offset(end)
    if not 0 <= start <= stop <= len(blob.data):
        raise ValueError(f"{what} is outside {blob.file_name}")
    return blob.data[start:stop]


def table_part(blob: Blob, name: str, address: int, raw: bytes, values: dict, file: str) -> Part:
    start = blob.offset(address)
    return Part(name, start, start + len(raw), file, Table(address, values, raw))


def text_table_names() -> dict[int, str]:
    """Named text tables from the MenuTextTable enum, without its count."""
    source = HEADER.read_text(encoding="ascii")
    match = re.search(r"typedef enum\s*\{([^}]+)\}\s*MenuTextTable;", source)
    if match is None:
        raise ValueError(f"{HEADER} has no MenuTextTable")
    names = {int(value): name for name, value in re.findall(r"(MENU_TEXT_\w+)\s*=\s*(\d+)", match[1])}
    return {index: name for index, name in names.items() if name != "MENU_TEXT_TABLE_COUNT"}


def text_table_count() -> int:
    source = HEADER.read_text(encoding="ascii")
    match = re.search(r"MENU_TEXT_TABLE_COUNT\s*=\s*(\d+)", source)
    if match is None:
        raise ValueError(f"{HEADER} has no MENU_TEXT_TABLE_COUNT")
    return int(match[1])


def content_type_names() -> dict[int, str]:
    """Content item classes with a #define in menu_internal.h, by high nibble."""
    source = HEADER.read_text(encoding="ascii")
    found = re.findall(r"#define MENU_CONTENT_ITEM_TYPE_(\w+)\s+0x([0-9A-F])000\b", source)
    return {int(nibble, 16): name.lower() for name, nibble in found if name != "MASK"}


def pad_button_names() -> dict[int, str]:
    """Pad bit names from the SDK header; aliases such as PADstart are skipped."""
    source = PAD_HEADER.read_text(encoding="ascii")
    names = {}
    for name, bit in re.findall(r"#define (PAD\w+)\s+\(1 << (\d+)\)", source):
        names.setdefault(1 << int(bit), name)
    return names


# ---------------------------------------------------------------------------
# Texture and sprites


def read_image(blob: Blob[MenuSymbols]) -> tuple[Part, Texture]:
    """MenuTimAsset: a three-word header, a 4-bit TIM and a second 256-color palette row."""
    address = blob.symbols.image
    header = read_bytes(blob, address, address + TIM_ASSET_HEADER.size, "menu texture header")
    count, tim_offset, clut_offset = TIM_ASSET_HEADER.unpack(header)
    clut_bytes = CLUT_ENTRY_COUNT * 2
    tim_size = 8 + 12 + clut_bytes + TIM_IMAGE_BLOCK_SIZE
    if (count, tim_offset, clut_offset) != (2, TIM_ASSET_HEADER.size, TIM_ASSET_HEADER.size + tim_size):
        raise ValueError("menu texture header does not describe one TIM and one extra palette row")
    raw = read_bytes(blob, address, address + clut_offset + clut_bytes, "menu texture")
    tim = parse_tim(raw[tim_offset:clut_offset])
    if tim.pixel_mode != 0 or tim.clut is None or len(tim.clut.payload) != clut_bytes:
        raise ValueError("menu TIM must have 4-bit pixels and 256 palette colors")
    if (tim.pixels.width_words * 4, tim.pixels.height) != (TEXTURE_SIZE, TEXTURE_SIZE):
        raise ValueError("menu TIM must be 256 x 256 pixels")
    rows = tim.clut.payload + raw[clut_offset:]
    palettes = {code: struct.unpack_from("<16H", rows, code * PALETTE_COLORS * 2) for code in range(32)}
    indices = tuple(nibble for byte in tim.pixels.payload for nibble in (byte & 15, byte >> 4))
    texture = Texture(indices, palettes)
    values = {"entry_count": count, "tim_offset": f"0x{tim_offset:X}", "second_palette_offset": f"0x{clut_offset:X}"}
    start = blob.offset(address)
    return Part("menu texture", start, start + len(raw), "image/image.yaml",
                MenuImage(address, raw, tim, texture, values)), texture


def read_icons(blob: Blob[MenuSymbols], texture: Texture) -> list[Part]:
    """Icon rectangles and their palette codes: two parallel arrays with the same count."""
    names = blob.symbols
    raw = read_bytes(blob, names.icon_sprites, names.icon_palettes, "icon sprites")
    if len(raw) % ICON_RECORD.size:
        raise ValueError("icon sprite table has a partial record")
    count = len(raw) // ICON_RECORD.size
    codes = read_bytes(blob, names.icon_palettes, names.icon_palettes + count, "icon palette codes")
    entries = []
    for index, ((u, v, width, height), code) in enumerate(zip(ICON_RECORD.iter_unpack(raw), codes)):
        if u + width > TEXTURE_SIZE or v + height > TEXTURE_SIZE:
            raise ValueError(f"icon {index} lies outside the menu texture")
        if code >> 4 > 1:
            raise ValueError(f"icon {index} uses palette row {code >> 4}; the menu uploads only two")
        entry = {"index": index, "u": u, "v": v, "width": width, "height": height,
                 "palette_code": f"0x{code:02X}", "clut_x": (code & 15) * 16, "clut_y": ICON_CLUT_Y_BASE + (code >> 4)}
        if width and height:
            entry["file"] = f"{index:03d}.png"
        entries.append(entry)
    start = blob.offset(names.icon_sprites)
    palette_start = blob.offset(names.icon_palettes)
    return [
        Part("icon sprites", start, start + len(raw), "icons/icons.yaml",
             Icons(names.icon_sprites, tuple(entries), texture, raw + codes)),
        Part("icon palette codes", palette_start, palette_start + count, note="Listed in icons/icons.yaml."),
    ]


def read_grid_sprites(blob: Blob[MenuSymbols], texture: Texture) -> Part:
    """The background grid: packed sprites menu_build_grid draws every frame."""
    address = blob.symbols.grid_sprites
    raw = read_bytes(blob, address, address + GRID_SPRITE_COUNT * GRID_SPRITE.size, "grid sprites")
    entries = []
    for index, (uv, reserved, x, y, width, height) in enumerate(GRID_SPRITE.iter_unpack(raw)):
        u, v = uv & 255, uv >> 8
        if u + width > TEXTURE_SIZE or v + height > TEXTURE_SIZE or width < 0 or height < 0:
            raise ValueError(f"grid sprite {index} lies outside the menu texture")
        entries.append({"index": index, "u": u, "v": v, "x": x, "y": y, "width": width, "height": height,
                        "palette_code": f"0x{int(index >= GRID_ALT_CLUT_START):02X}", "reserved": reserved})
    start = blob.offset(address)
    return Part("grid sprites", start, start + len(raw), "image/grid.yaml",
                GridSprites(address, tuple(entries), texture, raw))


def read_cursor_icons(blob: Blob[MenuSymbols]) -> Part:
    address = blob.symbols.cursor_icons
    if address + CURSOR_FRAMES > blob.symbols.variables:
        raise ValueError("cursor icon sequence runs into the runtime variables")
    raw = read_bytes(blob, address, address + CURSOR_FRAMES, "cursor icon ids")
    values = {"frames": list(raw), "note": "Icon ids from icons/icons.yaml, one per cursor animation frame."}
    return table_part(blob, "cursor icon ids", address, raw, values, "tables/cursor_icons.yaml")


# ---------------------------------------------------------------------------
# Text


def read_text(blob: Blob[MenuSymbols], chart: Chart | None) -> Part:
    """MenuTextResources: a table count, byte offsets, then u16-offset text tables."""
    address = blob.symbols.text
    raw = read_bytes(blob, address, blob.symbols.image, "text resource")
    count = text_table_count()
    header_size = 4 + count * 4
    if len(raw) < header_size or struct.unpack_from("<I", raw)[0] != count:
        raise ValueError(f"text resource must hold {count} tables")
    offsets = struct.unpack_from(f"<{count}I", raw, 4)
    if offsets[0] != header_size or list(offsets) != sorted(set(offsets)) or offsets[-1] >= len(raw):
        raise ValueError("invalid text table offsets")
    if blob.version == "jp" and chart is None:
        raise ValueError("JP MENU text needs CLOAD's character chart")
    names = text_table_names()
    codes = text_table.TWO_BYTE_CODES[blob.version]
    tables = []
    for index, (start, end) in enumerate(zip(offsets, (*offsets[1:], len(raw)))):
        # Parse against the whole resource: some unused entries point at the next table.
        parsed = text_table.parse(raw, start, codes)
        entries = []
        for entry in parsed.entries:
            value = {"index": entry.index, "offset": f"0x{entry.offset:X}",
                     "text": chart.decode(entry.data) if chart else entry.text, "bytes": entry.data.hex(" ")}
            if start + entry.offset >= end:
                value["outside_table"] = True
            entries.append(value)
        symbol = names.get(index)
        name = f"{index:02d}_{symbol.removeprefix('MENU_TEXT_').lower()}" if symbol else f"{index:02d}"
        table = {"index": index, "name": name, "offset": f"0x{start:X}", "size": f"0x{end - start:X}",
                 "address": hex_address(address + start), "entries": entries}
        if symbol:
            table["symbol"] = symbol
        tables.append(table)
    start = blob.offset(address)
    return Part("text resource", start, start + len(raw), "text/resource.yaml", TextResource(address, raw, tuple(tables)))


# ---------------------------------------------------------------------------
# Content layouts and small tables


def content_item(raw: bytes, types: dict[int, str]) -> dict[str, object]:
    packed_x, y, action_type, params = CONTENT_ITEM.unpack(raw)
    kind = packed_x >> CONTENT_TYPE_SHIFT
    item = {"x": packed_x & CONTENT_X_MASK, "style": (packed_x >> CONTENT_STYLE_SHIFT) & CONTENT_STYLE_MASK,
            "type": f"0x{kind:X}",
            "y": y, "action_type": action_type, "params": params.hex(" ")}
    if kind in types:
        item["type_name"] = types[kind]
    return item


def read_content(blob: Blob[MenuSymbols]) -> list[Part]:
    """Per-node content: group ids, item counts, action codes and the item sets they point at."""
    names = blob.symbols
    table_raw = read_bytes(blob, names.content_table, names.content_table + NODE_COUNT * 4, "content table")
    pointers = struct.unpack(f"<{NODE_COUNT}I", table_raw)
    group_raw = read_bytes(blob, names.group_ids, names.content_table, "content group ids")
    if len(group_raw) != NODE_COUNT:
        raise ValueError(f"expected {NODE_COUNT} content group ids")
    targets = sorted({value for value in pointers if value > CONTENT_LOAD_REQUEST})
    sets_end = names.text
    if not targets or targets[0] <= names.action_codes or targets[-1] >= sets_end:
        raise ValueError("content table points outside the content item sets")
    if names.default_items not in targets:
        raise ValueError("content item sets must include g_menu_default_content_items")
    action_raw = read_bytes(blob, names.action_codes, targets[0], "content action codes")
    if len(action_raw) % ACTION_ROW:
        raise ValueError("content action codes have a partial row")
    group_count = len(action_raw) // ACTION_ROW
    count_raw = read_bytes(blob, names.item_counts, names.item_counts + group_count, "content item counts")
    if not names.item_counts < names.default_count < names.item_counts + group_count <= names.action_codes:
        raise ValueError("default content count must lie inside the item-count table")
    if any(group >= group_count for group in group_raw):
        raise ValueError("a content group id has no item count")

    aliases: dict[int, list[str]] = {}
    for name, value in names.named.items():
        aliases.setdefault(value, []).append(name)
    types = content_type_names()
    sets = []
    for address, end in zip(targets, (*targets[1:], sets_end)):
        raw = read_bytes(blob, address, end, "content item set")
        if len(raw) % CONTENT_ITEM.size:
            raise ValueError(f"content item set at {hex_address(address)} has a partial item")
        users = [node for node, value in enumerate(pointers) if value == address]
        entry = {"address": hex_address(address), "stored_items": len(raw) // CONTENT_ITEM.size,
                 "nodes": users,
                 "items": [{"index": index, **content_item(raw[i:i + CONTENT_ITEM.size], types)}
                           for index, i in enumerate(range(0, len(raw), CONTENT_ITEM.size))]}
        if address in aliases:
            entry["symbols"] = sorted(aliases[address])
        sets.append(entry)
    sets_raw = read_bytes(blob, targets[0], sets_end, "content item sets")
    default_count = count_raw[names.default_count - names.item_counts] + 1

    nodes = []
    for node, (value, group) in enumerate(zip(pointers, group_raw)):
        entry = {"node": node, "group": group, "item_count": count_raw[group], "content": hex_address(value)}
        if value == 0:
            entry["content"] = None
        elif value == CONTENT_LOAD_REQUEST:
            entry["content"] = "load_request"
        nodes.append(entry)
    groups = [{"group": group, "item_count": count_raw[group],
               "action_codes": list(action_raw[group * ACTION_ROW:(group + 1) * ACTION_ROW])}
              for group in range(group_count)]
    return [
        table_part(blob, "content item counts", names.item_counts, count_raw,
                   {"groups": group_count, "counts": list(count_raw),
                    "default_count_index": names.default_count - names.item_counts,
                    "note": f"Entry {names.default_count - names.item_counts} is also "
                            f"{SYMBOL_NAMES['default_count']}. When g_menu_scene_type is -1, the menu reads that value "
                            f"plus one ({default_count}) items from {SYMBOL_NAMES['default_items']}."},
                   "content/item_counts.yaml"),
        table_part(blob, "content action codes", names.action_codes, action_raw,
                   {"groups": groups, "note": "Row = content group, column = the item's style field (bits 9-11)."},
                   "content/groups.yaml"),
        table_part(blob, "content item sets", targets[0], sets_raw,
                   {"record_format": "<" + CONTENT_ITEM.format.lstrip("<"), "sets": sets,
                    "note": "A set runs to the next address the content table uses. Items keep their stored order; "
                            "type is the high nibble of the packed X word."},
                   "content/item_sets.yaml"),
        table_part(blob, "content group ids", names.group_ids, group_raw,
                   {"group_ids": list(group_raw)}, "content/group_ids.yaml"),
        table_part(blob, "content table", names.content_table, table_raw,
                   {"nodes": nodes,
                    "note": f"Indexed by node number. The last twelve entries share memory with "
                            f"{SYMBOL_NAMES['scripts']} row 0, which the script player never reads."},
                   "content/nodes.yaml"),
    ]


def read_labels(blob: Blob[MenuSymbols], image_end: int) -> Part:
    """Label-id bytes read through several index bases, up to the group-id table."""
    named = blob.symbols.named
    start_address = named[LABEL_BASES[0]] + ACTION_LABEL_FIRST
    if start_address < image_end:
        raise ValueError("label ids overlap the menu texture")
    raw = read_bytes(blob, start_address, blob.symbols.group_ids, "label ids")
    bases = []
    for name in LABEL_BASES:
        address = named[name]
        if address - start_address >= len(raw):
            raise ValueError(f"{name} lies past the label ids")
        bases.append({"symbol": name, "address": hex_address(address), "first_index": start_address - address})
    values = {"bases": bases, "values": list(raw),
              "note": "The code indexes each base by an action type, node number or party-order slot. first_index is "
                      "the index that reads the first byte here; it is negative when the base lies inside these bytes. "
                      "Bases overlap, so one byte can serve several tables."}
    return table_part(blob, "label ids", start_address, raw, values, "tables/label_ids.yaml")


def read_scripts(blob: Blob[MenuSymbols]) -> Part:
    """Scripted pad input, one row per script id; row 0 is never played."""
    names = blob.symbols
    first = names.scripts + SCRIPT_ROW.size
    if first != names.content_table + NODE_COUNT * 4:
        raise ValueError("input script row 1 must follow the content table")
    raw = read_bytes(blob, first, names.grid_sprites, "input scripts")
    if not raw or len(raw) % SCRIPT_ROW.size:
        raise ValueError("input scripts have a partial row")
    buttons = pad_button_names()
    scripts = []
    for index, row in enumerate(SCRIPT_ROW.iter_unpack(raw), 1):
        terminated = SCRIPT_END in row
        length = row.index(SCRIPT_END) if terminated else len(row)
        frames = [[buttons.get(1 << bit, f"bit{bit}") for bit in range(16) if value >> bit & 1] for value in row[:length]]
        scripts.append({"script": index, "terminated": terminated, "frames": frames,
                        "words": [f"0x{value:04X}" for value in row]})
    values = {"scripts": scripts,
              "note": "One input per frame until 0xFFFF. An empty list is a frame with no buttons. "
                      "Scripts 1-3 also repeat the item-selection step when they end. "
                      "A row without 0xFFFF has no end inside its 24 words."}
    return table_part(blob, "input scripts", first, raw, values, "tables/input_scripts.yaml")


def read_variables(blob: Blob[MenuSymbols]) -> Part:
    start = blob.offset(blob.symbols.variables)
    if any(blob.data[start:]):
        return Part("runtime variables", start, len(blob.data), "unknown/variables.bin", blob.data[start:])
    return Part("runtime variables", start, len(blob.data),
                note="Menu state, node tree, window slots and memory-card buffers; all zero on disc, not exported.")


def read_blob(blob: Blob[MenuSymbols], chart: Chart | None = None) -> list[Part]:
    image, texture = read_image(blob)
    parts = read_icons(blob, texture)
    parts += read_content(blob)
    parts.append(read_text(blob, chart))
    parts.append(image)
    parts.append(read_labels(blob, blob.address + image.end))
    parts.append(read_scripts(blob))
    parts.append(read_grid_sprites(blob, texture))
    parts.append(read_cursor_icons(blob))
    parts.append(read_variables(blob))
    return cover_gaps(blob, sorted(parts, key=lambda part: part.start))


# ---------------------------------------------------------------------------
# Writing


@write_part.register
def _write_table(content: Table, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), **content.values, "bytes": content.raw.hex(" ")})


@write_part.register
def _write_image(content: MenuImage, path: Path) -> None:
    """The asset as stored, the TIM alone, and the whole texture through each palette."""
    path.parent.mkdir(parents=True, exist_ok=True)
    (path.parent / "asset.bin").write_bytes(content.raw)
    (path.parent / "image.tim").write_bytes(content.tim.to_bytes())
    previews = []
    for code, palette in content.texture.palettes.items():
        file = f"palette_{code:02X}.png"
        colors = content.texture.colors(code)
        png.write_rgba(path.parent / file, TEXTURE_SIZE, TEXTURE_SIZE, [colors[value] for value in content.texture.indices])
        previews.append({"palette_code": f"0x{code:02X}", "file": file, "colors": [f"0x{value:04X}" for value in palette]})
    dump_yaml(path, {
        "address": hex_address(content.address), "file": "asset.bin", "tim": "image.tim",
        "width": TEXTURE_SIZE, "height": TEXTURE_SIZE, **content.header,
        "stored_layout": content.tim.metadata(), "palettes": previews,
        "note": "menu_upload_graphics puts the pixels at (320, 0), the TIM's 256 colors in one row at (0, 498) "
                "and the second palette row at (0, 499). Palette code 0xRC is row R, column C. "
                "The upload sets the STP bit on nonzero colors; the stored words are unchanged here. "
                "Color zero is transparent in the PNGs and other colors are opaque.",
    })


@write_part.register
def _write_icons(content: Icons, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    for entry in content.entries:
        if "file" in entry:
            pixels = content.texture.crop(entry["u"], entry["v"], entry["width"], entry["height"],
                                          int(entry["palette_code"], 16))
            png.write_rgba(path.parent / entry["file"], entry["width"], entry["height"], pixels)
    dump_yaml(path, {
        "address": hex_address(content.address), "count": len(content.entries), "entries": list(content.entries),
        "note": "Each PNG crops the menu texture with the icon's own palette. Zero-size entries have no PNG.",
        "bytes": content.raw.hex(" "),
    })


@write_part.register
def _write_grid(content: GridSprites, path: Path) -> None:
    """Draw every grid sprite at its screen position, later sprites on top."""
    width = max(entry["x"] + entry["width"] for entry in content.entries)
    height = max(entry["y"] + entry["height"] for entry in content.entries)
    canvas = [(0, 0, 0, 0)] * (width * height)
    for entry in content.entries:
        pixels = content.texture.crop(entry["u"], entry["v"], entry["width"], entry["height"],
                                      int(entry["palette_code"], 16))
        for row in range(entry["height"]):
            target = (entry["y"] + row) * width + entry["x"]
            canvas[target:target + entry["width"]] = pixels[row * entry["width"]:(row + 1) * entry["width"]]
    path.parent.mkdir(parents=True, exist_ok=True)
    png.write_rgba(path.parent / "grid.png", width, height, canvas)
    dump_yaml(path, {
        "address": hex_address(content.address), "preview": "grid.png", "record_format": GRID_SPRITE.format,
        "entries": list(content.entries),
        "note": f"Sprites from {GRID_ALT_CLUT_START} on use palette 0x01; the rest use 0x00. "
                "The preview skips runtime tinting.",
        "bytes": content.raw.hex(" "),
    })


@write_part.register
def _write_text(content: TextResource, path: Path) -> None:
    tables = [{key: table[key] for key in ("index", "offset", "size") if key in table}
              | ({"symbol": table["symbol"]} if "symbol" in table else {}) | {"file": f"{table['name']}.yaml"}
              for table in content.tables]
    dump_yaml(path, {
        "address": hex_address(content.address), "file": "resource.bin", "table_count": len(content.tables),
        "tables": tables,
        "note": "Offsets are relative to the resource. Entries marked outside_table point past their table's end "
                "and read the start of the next table.",
    })
    (path.parent / "resource.bin").write_bytes(content.raw)
    for table in content.tables:
        dump_yaml(path.parent / f"{table['name']}.yaml", table)


# ---------------------------------------------------------------------------
# Putting it together


def load_blob(inputs: Inputs) -> Blob[MenuSymbols]:
    names = MenuSymbols.load(inputs.config / SYMBOL_FILE)
    files = splat_config.data_files(inputs.config / OVERLAY_CONFIG, inputs.assets)
    source = splat_config.file_containing(files, names.icon_sprites, SYMBOL_NAMES["icon_sprites"])
    if not source.path.exists():
        raise ValueError(f"{source.path} is missing; run make splat first")
    data = source.path.read_bytes()
    if len(data) != source.end - source.start:
        raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
    previous = source.start - 1
    for key in SYMBOL_NAMES:
        address = getattr(names, key)
        if not source.contains(address):
            raise ValueError(f"{SYMBOL_NAMES[key]} is outside {source.path}")
        if address <= previous:
            raise ValueError(f"{SYMBOL_NAMES[key]} is out of resource order")
        previous = address
    bases = [names.named[name] for name in LABEL_BASES]
    if bases != sorted(set(bases)) or not names.image < bases[0] < bases[-1] < names.group_ids:
        raise ValueError("label-id bases must lie in order between g_menu_tim and g_menu_content_group_ids")
    return Blob(data, source.start, source.path.name, inputs.version, names)


def load_text_chart(inputs: Inputs) -> Chart | None:
    if inputs.version != "jp":
        return None
    return Chart(cload.load_blob(cload.Inputs(inputs.version, inputs.config, inputs.assets)))


def extract(inputs: Inputs, output: Path) -> None:
    """Read everything, write a temporary folder, then move the finished export into place."""
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
            "address": hex_address(chart.blob.address + chart.start), "size": f"0x{chart.end - chart.start:X}",
            "note": "JP text uses CLOAD's character chart. MENU does not contain this conversion table.",
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
