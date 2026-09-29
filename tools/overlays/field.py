#!/usr/bin/env python3
"""Export FIELD's embedded images, text and tables as files people can read.

The build links one unchanged data blob. This tool opens it up into PNGs, YAML
and original resource files. Scene IMGs are separate; tools.scenes.field_scene
handles those.

Like the other overlay extractors, readers return Parts and the byte map covers
the whole blob. Small table layouts live in field_tables.py. Japanese text uses
CLOAD's copy of the character chart, also extracted by make splat.

Example:

    python3 -m tools.overlays.field --version us assets/exports/us/overlays/field
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import shutil
import struct
import sys
import tempfile

from tools.assets.psx_tim import parse_embedded_tim
from tools.overlays import cload, icon_set, png, splat_config, symbols, text_table
from tools.overlays.card_data import Chart, TextEntry, TextList
from tools.overlays.field_tables import TABLES
from tools.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[2]
OVERLAY_CONFIG = "overlays/FIELD.BIN.yaml"
SYMBOL_FILES = ("symbols/shared_symbol_addrs.txt", "symbols/field_symbol_addrs.txt")

# C layouts: field_actor_slot_resources.c, field_actor_tables.h, field_records.h,
# field_ability_progression.h, field_dialog_screens.c and golem_shape.h.
BUILTIN_ENTRY_COUNT = 256
BUILTIN_ENTRY = struct.Struct("<4HB7s")
ANIMATION_DEFINITION = struct.Struct("<4B8H4BH2B")
FIRST_ANIMATION = 2
ACTION_DESCRIPTOR = struct.Struct("<II")
ABILITY_RULE_COUNT = 18
TECHNIQUE_RULE_COUNT = 227
GOLEM_SHAPE_COUNT = 11
GOLEM_SHAPE_BYTES = 88
TITLE_CHOICE_INDEX = 19
MENU_WIDTH = 64
MENU_HEIGHT = 32
MENU_PALETTE_COUNT = 2
TRANSITION_SIZE = 32


# ---------------------------------------------------------------------------
# Inputs and decoded parts


@dataclass(frozen=True)
class FieldSymbols:
    """Resource anchors plus the complete symbol map for pointer table names."""

    pixels: int
    texture: int
    animations: int
    hud_colors: int
    title_choice: int
    abilities: int
    techniques: int
    save_codes: int
    technique_names: int
    command_names: int
    golem_palettes: int
    golem_portrait_palettes: int
    portrait_palettes: int
    item_names: int
    panel_quads: int
    menu_image: int
    weekdays: int
    windows: int
    actions: int
    levels: int
    item_name_table: int
    drop_handlers: int
    golem_shapes: int
    variables: int
    named: dict[str, int]

    @classmethod
    def load(cls, config: Path) -> FieldSymbols:
        named = {}
        for file in SYMBOL_FILES:
            named.update(symbols.load(config / file))
        required = set(SYMBOL_NAMES.values()) | {spec.symbol for spec in TABLES}
        required |= {"g_field_transition_tiles", "g_field_transition_tiles_alt"}
        missing = sorted(required - named.keys())
        if missing:
            raise ValueError(
                f"{config} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES in field.py or TABLES in field_tables.py."
            )
        return cls(**{key: named[name] for key, name in SYMBOL_NAMES.items()}, named=named)


SYMBOL_NAMES = {
    "pixels": "g_field_pixel_lookup_tables",
    "texture": "g_field_resource_buffer",
    "animations": "g_field_resource_blob",
    "hud_colors": "g_field_hud_hp_colors",
    "title_choice": "g_field_title_choice_text_entry",
    "abilities": "g_field_ability_unlock_rules",
    "techniques": "g_field_technique_unlock_rules",
    "save_codes": "g_field_known_save_codes",
    "technique_names": "g_field_technique_names",
    "command_names": "g_field_command_names",
    "golem_palettes": "g_field_golem_palettes",
    "golem_portrait_palettes": "g_field_golem_portrait_palettes",
    "portrait_palettes": "g_field_portrait_palettes",
    "item_names": "g_field_item_names",
    "panel_quads": "g_field_timed_panel_quads",
    "menu_image": "g_field_menu_frame_image",
    "weekdays": "g_field_weekday_names",
    "windows": "g_field_text_window_layouts",
    "actions": "g_field_default_action_bank",
    "levels": "g_field_monster_level_by_rank",
    "item_name_table": "g_field_item_name_table",
    "drop_handlers": "g_field_drop_handlers",
    "golem_shapes": "g_golem_shape_table",
    "variables": "g_field_fade_target",
}


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
class Image:
    """An indexed or direct-color texture and its original storage."""

    address: int
    width: int
    height: int
    pixels: bytes
    palettes: tuple[tuple[int, ...], ...]
    raw: bytes
    original_file: str
    metadata: dict[str, object]


@dataclass(frozen=True)
class Palettes:
    address: int
    colors: tuple[int, ...]


@dataclass(frozen=True)
class Resource:
    """A decoded directory and the original resource it describes."""

    address: int
    metadata: dict[str, object]
    raw: bytes


def read_bytes(blob: Blob, address: int, size: int, what: str) -> bytes:
    start = blob.offset(address)
    if size < 0 or start < 0 or start + size > len(blob.data):
        raise ValueError(f"{what} is outside {blob.file_name}")
    return blob.data[start : start + size]


def table_part(blob: Blob, name: str, address: int, raw: bytes, values: dict, file: str) -> Part:
    start = blob.offset(address)
    return Part(name, start, start + len(raw), file, Table(address, values, raw))


def repeated_word(blob: Blob, end: int, limit: int, name: str) -> list[Part]:
    if end >= 4 and end + 4 <= limit and blob.data[end : end + 4] == blob.data[end - 4 : end]:
        return [Part(name, end, end + 4, note="Repeats the resource's last word.")]
    return []


# ---------------------------------------------------------------------------
# Images and palettes


def read_common_texture(blob: Blob[FieldSymbols]) -> list[Part]:
    address = blob.symbols.texture
    raw = read_bytes(blob, address, blob.symbols.animations - address, "common TIM")
    tim = parse_embedded_tim(raw, trailing_duplicate_word=True)
    if tim.pixel_mode != 0 or tim.clut is None or len(tim.clut.payload) % 32:
        raise ValueError("common TIM must have 4-bit pixels and complete 16-color palettes")
    palettes = tuple(struct.iter_unpack("<16H", tim.clut.payload))
    content = Image(
        address, tim.pixels.width_words * 4, tim.pixels.height,
        tim.pixels.payload, palettes, tim.to_bytes(), "image.tim",
        {"stored_layout": tim.metadata(),
         "note": "Whole texture shown through each palette. Runtime sprites choose a palette per part; "
                 "FIELD uploads this texture at (384, 0) and its CLUT at (0, 490)."},
    )
    start = blob.offset(address)
    end = start + len(content.raw)
    return [Part("common texture", start, end, "images/common/image.yaml", content)] + repeated_word(
        blob, end, blob.offset(blob.symbols.animations), "common texture trailing word"
    )


def read_transition_tiles(blob: Blob[FieldSymbols]) -> list[Part]:
    """The two 32 x 32 fade tiles are stored together in one direct-color TIM."""
    address = blob.symbols.named["g_field_transition_tiles"] - 20
    limit = blob.symbols.named["g_field_target_filters"]
    raw = read_bytes(blob, address, limit - address, "transition TIM")
    tim = parse_embedded_tim(raw, trailing_duplicate_word=True)
    if tim.pixel_mode != 2 or tim.clut is not None:
        raise ValueError("transition TIM must use direct 16-bit color")
    if (tim.pixels.width_words, tim.pixels.height) != (TRANSITION_SIZE, TRANSITION_SIZE * 2):
        raise ValueError("transition TIM must contain two 32 x 32 tiles")
    if blob.symbols.named["g_field_transition_tiles_alt"] != address + 20 + TRANSITION_SIZE ** 2 * 2:
        raise ValueError("alternate transition tile does not start halfway through the TIM")
    content = Image(address, tim.pixels.width_words, tim.pixels.height, tim.pixels.payload,
                    (), tim.to_bytes(), "image.tim",
                    {"stored_layout": tim.metadata(), "note": "Top and bottom halves are the two alternating fade tiles."})
    start, end = blob.offset(address), blob.offset(address) + len(content.raw)
    return [Part("transition tiles", start, end, "images/transition/image.yaml", content)] + repeated_word(
        blob, end, blob.offset(limit), "transition tiles trailing word"
    )


def read_palette_header(blob: Blob, address: int, offsets: tuple[int, ...], name: str) -> Part:
    raw = read_bytes(blob, address, 4 + len(offsets) * 4, name + " header")
    values = struct.unpack("<" + "I" * (len(offsets) + 1), raw)
    if values != (len(offsets), *offsets):
        raise ValueError(f"invalid {name} offsets")
    return table_part(blob, name + " header", address, raw,
                      {"count": len(offsets), "offsets": list(offsets)}, f"palettes/{name}_header.yaml")


def read_palettes(blob: Blob[FieldSymbols]) -> list[Part]:
    parts = [
        read_palette_header(blob, blob.symbols.golem_palettes - 20, (20, 532, 1044, 1556), "golem"),
        read_palette_header(blob, blob.symbols.portrait_palettes - 12, (12, 44), "portrait"),
    ]
    # Both golem banks have two groups of sixteen 16-color palettes. The old
    # portrait databin held only the first group; the second followed in data_c.
    for key, count in (("golem_palettes", 512), ("golem_portrait_palettes", 512), ("portrait_palettes", 32)):
        address = getattr(blob.symbols, key)
        raw = read_bytes(blob, address, count * 2, key)
        content = Palettes(address, tuple(value for value, in struct.iter_unpack("<H", raw)))
        start = blob.offset(address)
        parts.append(Part(key, start, start + len(raw), f"palettes/{key}.yaml", content))
    for address, size, limit, name in (
        (blob.symbols.golem_portrait_palettes, 1024, blob.symbols.portrait_palettes - 12, "golem palettes trailing word"),
        (blob.symbols.portrait_palettes, 64, blob.symbols.item_names, "portrait palettes trailing word"),
    ):
        parts += repeated_word(blob, blob.offset(address) + size, blob.offset(limit), name)
    return parts


def read_menu_image(blob: Blob[FieldSymbols]) -> list[Part]:
    address = blob.symbols.menu_image
    palette_size = MENU_PALETTE_COUNT * 32
    size = palette_size + MENU_WIDTH * MENU_HEIGHT // 2
    raw = read_bytes(blob, address, size, "menu frame image")
    content = Image(address, MENU_WIDTH, MENU_HEIGHT, raw[palette_size:],
                    tuple(struct.iter_unpack("<16H", raw[:palette_size])), raw, "image.bin", {})
    start, end = blob.offset(address), blob.offset(address) + size
    header = read_palette_header(blob, address - 8, (8,), "menu_frame")
    return [header, Part("menu frame image", start, end, "images/menu_frame/image.yaml", content)] + repeated_word(
        blob, end, blob.offset(blob.symbols.weekdays), "menu frame trailing word"
    )


# ---------------------------------------------------------------------------
# Text and small tables


def read_text(blob: Blob[FieldSymbols], chart: Chart | None) -> list[Part]:
    names = blob.symbols
    tables = (
        ("ui", names.title_choice - TITLE_CHOICE_INDEX * 2, names.abilities),
        ("techniques", names.technique_names, names.command_names),
        ("commands", names.command_names, names.golem_palettes),
        ("items", names.item_names, names.panel_quads),
        ("weekdays", names.weekdays, names.windows),
        ("item_names", names.item_name_table, names.drop_handlers),
    )
    parts = []
    note = "# Original game text codes are kept beside each string.\n"
    if chart is not None:
        note += "# Japanese decoded with CLOAD's character chart; see byte-map.yaml.\n"
    for name, address, limit in tables:
        raw = read_bytes(blob, address, limit - address, name + " text")
        parsed = text_table.parse(raw, 0, text_table.TWO_BYTE_CODES[blob.version])
        slots_end = address + parsed.entries[0].offset
        slots = {value: key for key, value in names.named.items() if address <= value < slots_end}
        entries = tuple(
            TextEntry(entry.index, slots.get(address + entry.index * 2),
                      chart.decode(entry.data) if chart else entry.text, entry.data)
            for entry in parsed.entries
        )
        start = blob.offset(address)
        parts.append(Part(name + " text", start, start + parsed.size,
                          f"text/{name}.yaml", TextList(address, entries, note)))
    return parts


def read_small_tables(blob: Blob[FieldSymbols]) -> list[Part]:
    parts = []
    aliases: dict[int, list[str]] = {}
    for name, address in blob.symbols.named.items():
        aliases.setdefault(address, []).append(name)
    for spec in TABLES:
        address = blob.symbols.named[spec.symbol]
        layout = struct.Struct("<" + spec.format)
        raw = read_bytes(blob, address, layout.size * spec.count, spec.symbol)
        rows = []
        for values in layout.iter_unpack(raw):
            if spec.fields:
                row = dict(zip(spec.fields, values, strict=True))
            elif spec.resolve_symbols:
                value = values[0]
                row = {"value": hex_address(value)}
                if value in aliases:
                    row["symbols"] = sorted(aliases[value])
            else:
                row = values[0] if len(values) == 1 else list(values)
            rows.append(row)
        values = {"symbol": spec.symbol, "record_format": "<" + spec.format, "entries": rows}
        file = f"tables/{spec.symbol.removeprefix('g_')}.yaml"
        parts.append(table_part(blob, spec.symbol, address, raw, values, file))
    return parts


def read_unlock_rules(blob: Blob[FieldSymbols]) -> list[Part]:
    parts = []
    for key, count, prerequisites in (("abilities", ABILITY_RULE_COUNT, 2), ("techniques", TECHNIQUE_RULE_COUNT, 4)):
        address = getattr(blob.symbols, key)
        size = prerequisites * 2 + (1 if key == "abilities" else 3)
        raw = read_bytes(blob, address, count * size, key + " unlock rules")
        rules = []
        for index in range(count):
            row = raw[index * size : (index + 1) * size]
            rule = {"prerequisites": [
                {"ability": row[i * 2], "proficiency": row[i * 2 + 1]} for i in range(prerequisites)
            ]}
            if key == "abilities":
                rule["result"] = row[-1]
            else:
                rule.update(weapon=row[-3], result=row[-2], weapon_proficiency=row[-1])
            rules.append(rule)
        parts.append(table_part(blob, key + " unlock rules", address, raw,
                                {"rules": rules}, f"tables/{key}_unlock_rules.yaml"))
    return parts


def read_save_strings(blob: Blob[FieldSymbols]) -> list[Part]:
    names = blob.symbols
    address = names.techniques + TECHNIQUE_RULE_COUNT * 11
    raw = read_bytes(blob, address, names.save_codes - address, "save file names")
    strings = []
    cursor = 0
    while cursor < len(raw):
        if raw[cursor] == 0:
            cursor += 1
            continue
        end = raw.find(b"\x00", cursor)
        if end < 0:
            raise ValueError("save file name has no terminator before the product codes")
        strings.append({"address": hex_address(address + cursor), "text": raw[cursor:end].decode("ascii")})
        cursor = end + 1
    parts = [table_part(blob, "save file names", address, raw, {"strings": strings}, "text/save_files.yaml")]
    raw = read_bytes(blob, names.save_codes, 11 * 12, "known save codes")
    codes = [raw[i : i + 12].decode("ascii") for i in range(0, len(raw), 12)]
    parts.append(table_part(blob, "known save codes", names.save_codes, raw,
                            {"codes": codes}, "text/known_save_codes.yaml"))
    return parts


# ---------------------------------------------------------------------------
# Resources with their own record layouts


def read_animations(blob: Blob[FieldSymbols]) -> list[Part]:
    """Decode the built-in animation directory and definitions; keep track bytes intact."""
    address = blob.symbols.animations
    raw = read_bytes(blob, address, blob.symbols.hud_colors - address, "built-in animations")
    directory_size = BUILTIN_ENTRY_COUNT * BUILTIN_ENTRY.size
    if len(raw) < directory_size:
        raise ValueError("built-in animation directory is truncated")
    unknown, tracks, table, reserved = struct.unpack_from("<4I", raw)
    index_count = BUILTIN_ENTRY_COUNT - FIRST_ANIMATION
    if not directory_size <= tracks <= table <= len(raw) - index_count:
        raise ValueError("invalid built-in animation section offsets")
    indices = raw[table : table + index_count]
    definitions_start = table + index_count
    definition_count = max(indices) + 1
    end = definitions_start + definition_count * ANIMATION_DEFINITION.size
    if end > len(raw):
        raise ValueError("animation definitions run past the next resource")
    entries = []
    for index in range(1, BUILTIN_ENTRY_COUNT):
        parts, curves, segments, duration, count, extra = BUILTIN_ENTRY.unpack_from(raw, index * BUILTIN_ENTRY.size)
        if count and any(offset < directory_size or offset >= tracks for offset in (parts, curves, segments)):
            raise ValueError(f"animation {index} points outside its part and curve data")
        entry = {
            "index": index, "parts_offset": parts, "curves_offset": curves,
            "segments_offset": segments, "duration": duration, "part_count": count,
            "unknown_bytes": extra.hex(" "),
        }
        if index >= FIRST_ANIMATION:
            entry["definition"] = indices[index - FIRST_ANIMATION]
        entries.append(entry)
    definitions = []
    for index in range(definition_count):
        values = ANIMATION_DEFINITION.unpack_from(raw, definitions_start + index * ANIMATION_DEFINITION.size)
        definitions.append({
            "index": index, "vibration_curves": list(values[:2]),
            "sound_subtypes": list(values[2:4]), "sound_events": list(values[4:6]),
            "sound_commands": list(values[6:8]), "flags": f"0x{values[8]:04X}",
            "frame_sound": f"0x{values[9]:04X}", "unknown_10": values[10],
            "duration": values[11], "unknown_14_17": list(values[12:16]),
            "unknown_18": values[16], "unknown_1a_1b": list(values[17:]),
        })
    metadata = {
        "header": {"unknown_0": unknown, "track_data_offset": tracks,
                   "animation_table_offset": table, "unknown_c": reserved},
        "entries": entries, "definitions": definitions,
        "note": "The directory and animation definitions are decoded here. Part, curve and frame data "
                "remain in resource.bin at their original offsets; this is not an animation renderer.",
    }
    start = blob.offset(address)
    content = Resource(address, metadata, raw[:end])
    return [Part("built-in animations", start, start + end, "animations/builtin.yaml", content)] + repeated_word(
        blob, start + end, blob.offset(blob.symbols.hud_colors), "animation resource trailing word"
    )


def read_action_bank(blob: Blob[FieldSymbols]) -> Part:
    """Four offset-indexed banks of eight-byte FieldActionDescriptors."""
    address = blob.symbols.actions
    raw = read_bytes(blob, address, blob.symbols.levels - address, "default action bank")
    if len(raw) < 16:
        raise ValueError("action bank header is truncated")
    offsets = struct.unpack_from("<4I", raw)
    if offsets[0] != 16 or list(offsets) != sorted(set(offsets)) or offsets[-1] >= len(raw):
        raise ValueError("invalid action bank offsets")
    tables = []
    for index, (start, end) in enumerate(zip(offsets, (*offsets[1:], len(raw)))):
        if (end - start) % ACTION_DESCRIPTOR.size:
            raise ValueError("action bank section has a partial descriptor")
        entries = []
        for info, params in ACTION_DESCRIPTOR.iter_unpack(raw[start:end]):
            entries.append({
                "info": hex_address(info), "params": hex_address(params),
                "kind": info & 15, "side_rule": (info >> 4) & 3,
                "defense_slot": (info >> 6) & 3, "status_intensity_shift": (info >> 8) & 7,
                "unknown_1": (info >> 8) & 255, "power": (info >> 16) & 255, "handler": info >> 24,
                "attack_stat": params & 15, "defense_stat": (params >> 4) & 15,
            })
        tables.append({"index": index, "offset": start, "entries": entries})
    content = Resource(address, {"tables": tables, "note": "Parameter bits beyond the two stat selectors depend on the handler."}, raw)
    start = blob.offset(address)
    return Part("default action bank", start, start + len(raw), "actions/default.yaml", content)


def read_golem_shapes(blob: Blob[FieldSymbols]) -> Part:
    address = blob.symbols.golem_shapes
    raw = read_bytes(blob, address, GOLEM_SHAPE_COUNT * GOLEM_SHAPE_BYTES, "golem shapes")
    shapes = []
    for index in range(GOLEM_SHAPE_COUNT):
        start = index * GOLEM_SHAPE_BYTES
        count, reserved, width, height, x, y = struct.unpack_from("<4B2h", raw, start)
        rotations = []
        for rotation in range(4):
            points = [
                {"x": px, "y": py, "glyph_id": glyph}
                for px, py, glyph in struct.iter_unpack("<bbh", raw[start + 8 + rotation * 20 : start + 28 + rotation * 20])
            ]
            rotations.append({"origin": points[0], "parts": points[1:]})
        shapes.append({"index": index, "count": count, "reserved": reserved,
                       "grid_width": width, "grid_height": height, "origin_x": x, "origin_y": y,
                       "rotations": rotations})
    return table_part(blob, "golem shapes", address, raw, {"shapes": shapes}, "tables/golem_shapes.yaml")


def read_panel_quads(blob: Blob[FieldSymbols]) -> Part:
    address = blob.symbols.panel_quads
    raw = read_bytes(blob, address, 12 * 12, "timed panel quads")
    fields = ("x", "y", "u", "v", "width", "height", "flags")
    quads = [dict(zip(fields, values)) for values in struct.iter_unpack("<HHBBHHH", raw)]
    return table_part(blob, "timed panel quads", address, raw, {"quads": quads}, "tables/timed_panel_quads.yaml")


def read_variables(blob: Blob[FieldSymbols]) -> Part:
    start = blob.offset(blob.symbols.variables)
    if any(blob.data[start:]):
        return Part("variables", start, len(blob.data), "unknown/variables.bin", blob.data[start:])
    return Part("variables", start, len(blob.data), note="FIELD's runtime variables and buffers; all zero on disc, not exported.")


def read_blob(blob: Blob[FieldSymbols], chart: Chart | None = None) -> list[Part]:
    """Collect the known resources in memory order and preserve all remaining bytes."""
    parts = read_common_texture(blob)
    parts += read_animations(blob)
    parts += read_small_tables(blob)
    parts += read_transition_tiles(blob)
    parts += read_text(blob, chart)
    parts += read_unlock_rules(blob)
    parts += read_save_strings(blob)
    parts += read_palettes(blob)
    parts.append(read_panel_quads(blob))
    parts += read_menu_image(blob)
    parts.append(read_action_bank(blob))
    parts.append(read_golem_shapes(blob))
    parts.append(read_variables(blob))
    return cover_gaps(blob, sorted(parts, key=lambda part: part.start))


# ---------------------------------------------------------------------------
# Writing


@write_part.register
def _write_table(content: Table, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), **content.values, "bytes": content.raw.hex(" ")})


@write_part.register
def _write_resource(content: Resource, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), "file": "resource.bin", **content.metadata})
    (path.parent / "resource.bin").write_bytes(content.raw)


@write_part.register
def _write_image(content: Image, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    (path.parent / content.original_file).write_bytes(content.raw)
    previews = []
    for index, palette in enumerate(content.palettes or ((),)):
        if palette:
            colors = [icon_set.bgr555_to_rgba(value) for value in palette]
            pixels = [colors[nibble] for byte in content.pixels for nibble in (byte & 15, byte >> 4)]
            file = f"palette_{index:02X}.png"
        else:
            pixels = [icon_set.bgr555_to_rgba(value) for value, in struct.iter_unpack("<H", content.pixels)]
            file = "image.png"
        png.write_rgba(path.parent / file, content.width, content.height, pixels)
        preview = {"file": file}
        if palette:
            preview.update(palette=index, colors=[f"0x{value:04X}" for value in palette])
        previews.append(preview)
    dump_yaml(path, {
        "address": hex_address(content.address), "file": content.original_file,
        "width": content.width, "height": content.height, "previews": previews,
        "transparency": "BGR555 value zero is transparent. Other colors are shown opaque; the original words retain STP bits.",
        **content.metadata,
    })


@write_part.register
def _write_palettes(content: Palettes, path: Path) -> None:
    """Swatches use 8-pixel squares, sixteen colors per row."""
    rows = [content.colors[i : i + 16] for i in range(0, len(content.colors), 16)]
    pixels = []
    for row in rows:
        strip = [icon_set.bgr555_to_rgba(value) for value in row for _ in range(8)]
        pixels.extend(strip * 8)
    dump_yaml(path, {
        "address": hex_address(content.address), "preview": path.with_suffix(".png").name,
        "palettes": [[f"0x{value:04X}" for value in row] for row in rows],
    })
    png.write_rgba(path.with_suffix(".png"), 128, len(rows) * 8, pixels)


# ---------------------------------------------------------------------------
# Putting it together


def load_blob(inputs: Inputs) -> Blob[FieldSymbols]:
    names = FieldSymbols.load(inputs.config)
    files = splat_config.data_files(inputs.config / OVERLAY_CONFIG, inputs.assets)
    source = splat_config.file_containing(files, names.pixels, SYMBOL_NAMES["pixels"])
    if not source.path.exists():
        raise ValueError(f"{source.path} is missing; run make splat first")
    data = source.path.read_bytes()
    if len(data) != source.end - source.start:
        raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
    for key in SYMBOL_NAMES:
        if not source.contains(getattr(names, key)):
            raise ValueError(f"{SYMBOL_NAMES[key]} is outside {source.path}")
    for spec in TABLES:
        if not source.contains(names.named[spec.symbol]):
            raise ValueError(f"{spec.symbol} is outside {source.path}")
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
            "note": "FIELD uses the same game text codes. The Shift-JIS conversion chart lives in CLOAD, outside this blob.",
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
