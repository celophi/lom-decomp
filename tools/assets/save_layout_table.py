#!/usr/bin/env python3
"""Extract and rebuild TITLE save-layout primitive descriptor tables."""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml


FORMAT_NAME = "title_save_layout_table"
FORMAT_VERSION = 1
RECORD_SIZE = 0x18
PRIMITIVE_NAMES = {
    0: "hidden",
    1: "sprite_strip",
    2: "tile",
    3: "panel_quad",
    4: "slot_sprite",
}
PRIMITIVE_VALUES = {name: value for value, name in PRIMITIVE_NAMES.items()}


class SaveLayoutTableError(ValueError):
    """Raised when a save-layout table or its YAML document is invalid."""


@dataclass(frozen=True)
class SaveLayoutEntry:
    apply_slide: bool
    semi_transparent: bool
    blend_mode: int
    primitive: str
    texture: str
    x: int
    y: int
    tile_x: int
    tile_y: int
    u: int
    v: int
    width: int
    height: int


@dataclass(frozen=True)
class SaveLayoutTable:
    textures: tuple[str, ...]
    entries: tuple[SaveLayoutEntry, ...]

    @classmethod
    def parse_binary(
        cls,
        data: bytes,
        texture_names: tuple[str, ...],
        expected_count: int | None = None,
    ) -> "SaveLayoutTable":
        textures = _validate_textures(texture_names)
        if len(data) % RECORD_SIZE != 0:
            raise SaveLayoutTableError(
                f"binary size 0x{len(data):X} is not divisible by record size 0x18"
            )
        count = len(data) // RECORD_SIZE
        if expected_count is not None and count != expected_count:
            raise SaveLayoutTableError(
                f"binary contains {count} entries; expected {expected_count}"
            )

        entries = []
        for index in range(count):
            values = struct.unpack_from("<BBBBhhhhHHHHI", data, index * RECORD_SIZE)
            flags, primitive, texture_slot, padding = values[:4]
            if flags & ~0xF:
                raise SaveLayoutTableError(
                    f"entry {index} has unsupported flag bits 0x{flags & ~0xF:02X}"
                )
            if primitive not in PRIMITIVE_NAMES:
                raise SaveLayoutTableError(
                    f"entry {index} has unsupported primitive value {primitive}"
                )
            if texture_slot >= len(textures):
                raise SaveLayoutTableError(
                    f"entry {index} texture slot {texture_slot} is out of range"
                )
            if padding != 0:
                raise SaveLayoutTableError(
                    f"entry {index} padding byte must be zero, got 0x{padding:02X}"
                )
            if values[12] != 0:
                raise SaveLayoutTableError(
                    f"entry {index} reserved word must be zero, got 0x{values[12]:08X}"
                )
            entries.append(
                SaveLayoutEntry(
                    bool(flags & 1),
                    bool(flags & 2),
                    (flags >> 2) & 3,
                    PRIMITIVE_NAMES[primitive],
                    textures[texture_slot],
                    *values[4:12],
                )
            )
        return cls(textures, tuple(entries))

    @classmethod
    def parse_document(
        cls,
        document: object,
        expected_count: int | None = None,
        expected_textures: tuple[str, ...] | None = None,
    ) -> "SaveLayoutTable":
        if not isinstance(document, dict):
            raise SaveLayoutTableError("YAML root must be a mapping")
        expected_keys = {"format", "version", "entry_count", "textures", "entries"}
        if set(document) != expected_keys:
            raise SaveLayoutTableError(
                "YAML must contain format, version, entry_count, textures, and entries"
            )
        if document["format"] != FORMAT_NAME:
            raise SaveLayoutTableError(f"format must be '{FORMAT_NAME}'")
        if document["version"] != FORMAT_VERSION:
            raise SaveLayoutTableError(f"version must be {FORMAT_VERSION}")
        count = _validate_u32("entry_count", document["entry_count"])
        raw_textures = document["textures"]
        if not isinstance(raw_textures, list):
            raise SaveLayoutTableError("textures must be a sequence")
        textures = _validate_textures(tuple(raw_textures))
        if expected_textures is not None and textures != expected_textures:
            raise SaveLayoutTableError("YAML textures differ from the Splat configuration")
        raw_entries = document["entries"]
        if not isinstance(raw_entries, list):
            raise SaveLayoutTableError("entries must be a sequence")
        if len(raw_entries) != count:
            raise SaveLayoutTableError(
                f"YAML contains {len(raw_entries)} entries, but entry_count is {count}"
            )
        if expected_count is not None and count != expected_count:
            raise SaveLayoutTableError(
                f"YAML contains {count} entries; expected {expected_count}"
            )

        entry_keys = {
            "flags",
            "primitive",
            "texture",
            "position",
            "tile_position",
            "uv",
            "size",
        }
        entries = []
        for index, raw_entry in enumerate(raw_entries):
            if not isinstance(raw_entry, dict) or set(raw_entry) != entry_keys:
                raise SaveLayoutTableError(f"entries[{index}] has an invalid schema")
            flags = _parse_flags(index, raw_entry["flags"])
            primitive = raw_entry["primitive"]
            if primitive not in PRIMITIVE_VALUES:
                raise SaveLayoutTableError(
                    f"entries[{index}].primitive must be one of: "
                    + ", ".join(PRIMITIVE_VALUES)
                )
            texture = _validate_name(f"entries[{index}].texture", raw_entry["texture"])
            if texture not in textures:
                raise SaveLayoutTableError(
                    f"entries[{index}].texture {texture!r} is not listed in textures"
                )
            position = _parse_coord(index, "position", raw_entry["position"], signed=True)
            tile_position = _parse_coord(
                index, "tile_position", raw_entry["tile_position"], signed=True
            )
            uv = _parse_coord(index, "uv", raw_entry["uv"], signed=False)
            size = _parse_size(index, raw_entry["size"])
            entries.append(
                SaveLayoutEntry(
                    flags[0],
                    flags[1],
                    flags[2],
                    primitive,
                    texture,
                    position[0],
                    position[1],
                    tile_position[0],
                    tile_position[1],
                    uv[0],
                    uv[1],
                    size[0],
                    size[1],
                )
            )
        return cls(textures, tuple(entries))

    def to_bytes(self) -> bytes:
        textures = _validate_textures(self.textures)
        output = bytearray()
        for index, entry in enumerate(self.entries):
            flags = _encode_flags(index, entry)
            primitive = PRIMITIVE_VALUES.get(entry.primitive)
            if primitive is None:
                raise SaveLayoutTableError(
                    f"entries[{index}].primitive is invalid: {entry.primitive!r}"
                )
            if entry.texture not in textures:
                raise SaveLayoutTableError(
                    f"entries[{index}].texture {entry.texture!r} is not listed in textures"
                )
            values = (
                _validate_s16(f"entries[{index}].position.x", entry.x),
                _validate_s16(f"entries[{index}].position.y", entry.y),
                _validate_s16(f"entries[{index}].tile_position.x", entry.tile_x),
                _validate_s16(f"entries[{index}].tile_position.y", entry.tile_y),
                _validate_u16(f"entries[{index}].uv.u", entry.u),
                _validate_u16(f"entries[{index}].uv.v", entry.v),
                _validate_u16(f"entries[{index}].size.width", entry.width),
                _validate_u16(f"entries[{index}].size.height", entry.height),
            )
            output.extend(
                struct.pack(
                    "<BBBBhhhhHHHHI",
                    flags,
                    primitive,
                    textures.index(entry.texture),
                    0,
                    *values,
                    0,
                )
            )
        return bytes(output)

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "entry_count": len(self.entries),
            "textures": list(self.textures),
            "entries": [
                {
                    "flags": {
                        "apply_slide": entry.apply_slide,
                        "semi_transparent": entry.semi_transparent,
                        "blend_mode": entry.blend_mode,
                    },
                    "primitive": entry.primitive,
                    "texture": entry.texture,
                    "position": {"x": entry.x, "y": entry.y},
                    "tile_position": {"x": entry.tile_x, "y": entry.tile_y},
                    "uv": {"u": entry.u, "v": entry.v},
                    "size": {"width": entry.width, "height": entry.height},
                }
                for entry in self.entries
            ],
        }


def _validate_name(label: str, value: object) -> str:
    if not isinstance(value, str) or not value:
        raise SaveLayoutTableError(f"{label} must be a nonempty string")
    return value


def _validate_textures(values: tuple[object, ...]) -> tuple[str, ...]:
    textures = tuple(
        _validate_name(f"textures[{index}]", value)
        for index, value in enumerate(values)
    )
    if len(textures) > 0x100:
        raise SaveLayoutTableError("textures cannot contain more than 256 names")
    if len(set(textures)) != len(textures):
        raise SaveLayoutTableError("textures must contain unique names")
    return textures


def _validate_bool(label: str, value: object) -> bool:
    if type(value) is not bool:
        raise SaveLayoutTableError(f"{label} must be true or false")
    return value


def _validate_s16(label: str, value: object) -> int:
    if type(value) is not int or not -0x8000 <= value <= 0x7FFF:
        raise SaveLayoutTableError(f"{label} must be a signed 16-bit integer")
    return value


def _validate_u16(label: str, value: object) -> int:
    if type(value) is not int or not 0 <= value <= 0xFFFF:
        raise SaveLayoutTableError(f"{label} must be an unsigned 16-bit integer")
    return value


def _validate_u32(label: str, value: object) -> int:
    if type(value) is not int or not 0 <= value <= 0xFFFFFFFF:
        raise SaveLayoutTableError(f"{label} must be an unsigned 32-bit integer")
    return value


def _parse_flags(index: int, value: object) -> tuple[bool, bool, int]:
    if not isinstance(value, dict) or set(value) != {
        "apply_slide",
        "semi_transparent",
        "blend_mode",
    }:
        raise SaveLayoutTableError(f"entries[{index}].flags has an invalid schema")
    blend_mode = value["blend_mode"]
    if type(blend_mode) is not int or not 0 <= blend_mode <= 3:
        raise SaveLayoutTableError(
            f"entries[{index}].flags.blend_mode must be an integer in [0, 3]"
        )
    return (
        _validate_bool(f"entries[{index}].flags.apply_slide", value["apply_slide"]),
        _validate_bool(
            f"entries[{index}].flags.semi_transparent", value["semi_transparent"]
        ),
        blend_mode,
    )


def _encode_flags(index: int, entry: SaveLayoutEntry) -> int:
    apply_slide = _validate_bool(
        f"entries[{index}].flags.apply_slide", entry.apply_slide
    )
    semi_transparent = _validate_bool(
        f"entries[{index}].flags.semi_transparent", entry.semi_transparent
    )
    if type(entry.blend_mode) is not int or not 0 <= entry.blend_mode <= 3:
        raise SaveLayoutTableError(
            f"entries[{index}].flags.blend_mode must be an integer in [0, 3]"
        )
    return int(apply_slide) | (int(semi_transparent) << 1) | (entry.blend_mode << 2)


def _parse_coord(
    index: int, label: str, value: object, *, signed: bool
) -> tuple[int, int]:
    keys = {"x", "y"} if label != "uv" else {"u", "v"}
    if not isinstance(value, dict) or set(value) != keys:
        raise SaveLayoutTableError(
            f"entries[{index}].{label} must contain {', '.join(sorted(keys))}"
        )
    names = ("x", "y") if label != "uv" else ("u", "v")
    validator = _validate_s16 if signed else _validate_u16
    return tuple(
        validator(f"entries[{index}].{label}.{name}", value[name]) for name in names
    )


def _parse_size(index: int, value: object) -> tuple[int, int]:
    if not isinstance(value, dict) or set(value) != {"width", "height"}:
        raise SaveLayoutTableError(
            f"entries[{index}].size must contain width and height"
        )
    return (
        _validate_u16(f"entries[{index}].size.width", value["width"]),
        _validate_u16(f"entries[{index}].size.height", value["height"]),
    )


def dump_save_layout_table_yaml(table: SaveLayoutTable) -> str:
    return yaml.safe_dump(
        table.document(), sort_keys=False, allow_unicode=False, width=100
    )


def load_save_layout_table_yaml(path: Path) -> SaveLayoutTable:
    try:
        document = yaml.safe_load(path.read_text(encoding="ascii"))
    except UnicodeDecodeError as error:
        raise SaveLayoutTableError(f"{path} must contain ASCII text") from error
    except yaml.YAMLError as error:
        raise SaveLayoutTableError(f"invalid YAML in {path}: {error}") from error
    return SaveLayoutTable.parse_document(document)


def validate_roundtrip(yaml_path: Path, binary_path: Path) -> SaveLayoutTable:
    table = load_save_layout_table_yaml(yaml_path)
    if table.to_bytes() != binary_path.read_bytes():
        raise SaveLayoutTableError(f"{yaml_path} rebuild differs from {binary_path}")
    return table


def _summary(path: Path, table: SaveLayoutTable) -> str:
    return f"{path}: {len(table.entries)} entries, 0x{len(table.to_bytes()):X} bytes"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    build_parser = subparsers.add_parser("build")
    build_parser.add_argument("source", type=Path)
    build_parser.add_argument("output", type=Path)
    validate_parser = subparsers.add_parser("validate")
    validate_parser.add_argument("paths", nargs="+", type=Path)
    roundtrip_parser = subparsers.add_parser("roundtrip")
    roundtrip_parser.add_argument("source", type=Path)
    roundtrip_parser.add_argument("binary", type=Path)
    args = parser.parse_args()

    try:
        if args.command == "build":
            table = load_save_layout_table_yaml(args.source)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(table.to_bytes())
        elif args.command == "validate":
            for path in args.paths:
                table = load_save_layout_table_yaml(path)
                print(f"{_summary(path, table)} [valid]")
        elif args.command == "roundtrip":
            table = validate_roundtrip(args.source, args.binary)
            print(f"{_summary(args.source, table)} [round-trip OK]")
    except (OSError, SaveLayoutTableError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
