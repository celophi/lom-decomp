#!/usr/bin/env python3
"""Extract and rebuild TITLE's starting-weapon equipment records."""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml


FORMAT_NAME = "lom_starting_weapon_table"
FORMAT_VERSION = 1
RECORD_SIZE = 0x40
NAME_SIZE = 0x14
OPAQUE_TAIL_SIZE = 0x1A
KIND_MASK = 0x00000300
CATEGORY_MASK = 0x0000FC00
MATERIAL_MASK = 0x003F0000
KNOWN_ATTRIBUTE_MASK = KIND_MASK | CATEGORY_MASK | MATERIAL_MASK
UNKNOWN_ATTRIBUTE_MASK = 0xFFFFFFFF ^ KNOWN_ATTRIBUTE_MASK
WEAPON_KIND = 0


class _HexInt(int):
    pass


class _StartingWeaponDumper(yaml.SafeDumper):
    pass


_StartingWeaponDumper.add_representer(
    _HexInt,
    lambda dumper, value: dumper.represent_scalar(
        "tag:yaml.org,2002:int", f"0x{int(value):08X}"
    ),
)


class StartingWeaponTableError(ValueError):
    """Raised when a starting-weapon table or its YAML document is invalid."""


@dataclass(frozen=True)
class StartingWeaponEntry:
    name: str
    category: str
    material_id: int
    unknown_attribute_bits: int
    unknown_word_18: int
    unknown_word_1c: int
    unknown_word_20: int
    primary_value: int
    opaque_tail: bytes


@dataclass(frozen=True)
class StartingWeaponTable:
    categories: tuple[str, ...]
    entries: tuple[StartingWeaponEntry, ...]

    @classmethod
    def parse_binary(
        cls,
        data: bytes,
        category_names: tuple[str, ...],
        expected_count: int | None = None,
    ) -> "StartingWeaponTable":
        categories = _validate_categories(category_names)
        if len(data) % RECORD_SIZE != 0:
            raise StartingWeaponTableError(
                f"binary size 0x{len(data):X} is not divisible by record size 0x40"
            )
        count = len(data) // RECORD_SIZE
        if expected_count is not None and count != expected_count:
            raise StartingWeaponTableError(
                f"binary contains {count} records; expected {expected_count}"
            )

        entries = []
        for index in range(count):
            record = data[index * RECORD_SIZE : (index + 1) * RECORD_SIZE]
            name = _decode_name(index, record[:NAME_SIZE])
            attributes, word_18, word_1c, word_20, primary_value = struct.unpack_from(
                "<IIIIH", record, NAME_SIZE
            )
            kind = (attributes & KIND_MASK) >> 8
            if kind != WEAPON_KIND:
                raise StartingWeaponTableError(
                    f"record {index} equipment kind is {kind}; expected weapon kind 0"
                )
            category_index = (attributes & CATEGORY_MASK) >> 10
            if category_index >= len(categories):
                raise StartingWeaponTableError(
                    f"record {index} category {category_index} is out of range"
                )
            entries.append(
                StartingWeaponEntry(
                    name,
                    categories[category_index],
                    (attributes & MATERIAL_MASK) >> 16,
                    attributes & UNKNOWN_ATTRIBUTE_MASK,
                    word_18,
                    word_1c,
                    word_20,
                    primary_value,
                    record[0x26:0x40],
                )
            )
        return cls(categories, tuple(entries))

    @classmethod
    def parse_document(
        cls,
        document: object,
        expected_count: int | None = None,
        expected_categories: tuple[str, ...] | None = None,
    ) -> "StartingWeaponTable":
        if not isinstance(document, dict):
            raise StartingWeaponTableError("YAML root must be a mapping")
        expected_keys = {"format", "version", "record_count", "categories", "records"}
        if set(document) != expected_keys:
            raise StartingWeaponTableError(
                "YAML must contain format, version, record_count, categories, and records"
            )
        if document["format"] != FORMAT_NAME:
            raise StartingWeaponTableError(f"format must be '{FORMAT_NAME}'")
        if document["version"] != FORMAT_VERSION:
            raise StartingWeaponTableError(f"version must be {FORMAT_VERSION}")
        count = _validate_u32("record_count", document["record_count"])
        raw_categories = document["categories"]
        if not isinstance(raw_categories, list):
            raise StartingWeaponTableError("categories must be a sequence")
        categories = _validate_categories(tuple(raw_categories))
        if expected_categories is not None and categories != expected_categories:
            raise StartingWeaponTableError(
                "YAML categories differ from the Splat configuration"
            )
        raw_records = document["records"]
        if not isinstance(raw_records, list):
            raise StartingWeaponTableError("records must be a sequence")
        if len(raw_records) != count:
            raise StartingWeaponTableError(
                f"YAML contains {len(raw_records)} records, but record_count is {count}"
            )
        if expected_count is not None and count != expected_count:
            raise StartingWeaponTableError(
                f"YAML contains {count} records; expected {expected_count}"
            )

        record_keys = {
            "name",
            "category",
            "material_id",
            "unknown_attribute_bits",
            "unknown_words",
            "primary_value",
            "opaque_26_3f",
        }
        entries = []
        for index, raw_record in enumerate(raw_records):
            if not isinstance(raw_record, dict) or set(raw_record) != record_keys:
                raise StartingWeaponTableError(f"records[{index}] has an invalid schema")
            name = _validate_name(index, raw_record["name"])
            category = _validate_category(index, raw_record["category"], categories)
            unknown_words = raw_record["unknown_words"]
            if not isinstance(unknown_words, dict) or set(unknown_words) != {
                "offset_18",
                "offset_1c",
                "offset_20",
            }:
                raise StartingWeaponTableError(
                    f"records[{index}].unknown_words has an invalid schema"
                )
            unknown_bits = _validate_u32(
                f"records[{index}].unknown_attribute_bits",
                raw_record["unknown_attribute_bits"],
            )
            if unknown_bits & KNOWN_ATTRIBUTE_MASK:
                raise StartingWeaponTableError(
                    f"records[{index}].unknown_attribute_bits overlaps decoded fields"
                )
            entries.append(
                StartingWeaponEntry(
                    name,
                    category,
                    _validate_int(
                        f"records[{index}].material_id",
                        raw_record["material_id"],
                        0x3F,
                    ),
                    unknown_bits,
                    _validate_u32(
                        f"records[{index}].unknown_words.offset_18",
                        unknown_words["offset_18"],
                    ),
                    _validate_u32(
                        f"records[{index}].unknown_words.offset_1c",
                        unknown_words["offset_1c"],
                    ),
                    _validate_u32(
                        f"records[{index}].unknown_words.offset_20",
                        unknown_words["offset_20"],
                    ),
                    _validate_int(
                        f"records[{index}].primary_value",
                        raw_record["primary_value"],
                        0xFFFF,
                    ),
                    _parse_hex_bytes(
                        f"records[{index}].opaque_26_3f",
                        raw_record["opaque_26_3f"],
                        OPAQUE_TAIL_SIZE,
                    ),
                )
            )
        return cls(categories, tuple(entries))

    def to_bytes(self) -> bytes:
        categories = _validate_categories(self.categories)
        output = bytearray()
        for index, entry in enumerate(self.entries):
            category = _validate_category(index, entry.category, categories)
            material_id = _validate_int(
                f"records[{index}].material_id", entry.material_id, 0x3F
            )
            unknown_bits = _validate_u32(
                f"records[{index}].unknown_attribute_bits",
                entry.unknown_attribute_bits,
            )
            if unknown_bits & KNOWN_ATTRIBUTE_MASK:
                raise StartingWeaponTableError(
                    f"records[{index}].unknown_attribute_bits overlaps decoded fields"
                )
            attributes = (
                unknown_bits
                | (categories.index(category) << 10)
                | (material_id << 16)
            )
            output.extend(_encode_name(index, entry.name))
            output.extend(
                struct.pack(
                    "<IIIIH",
                    attributes,
                    _validate_u32(f"records[{index}].unknown_word_18", entry.unknown_word_18),
                    _validate_u32(f"records[{index}].unknown_word_1c", entry.unknown_word_1c),
                    _validate_u32(f"records[{index}].unknown_word_20", entry.unknown_word_20),
                    _validate_int(f"records[{index}].primary_value", entry.primary_value, 0xFFFF),
                )
            )
            if len(entry.opaque_tail) != OPAQUE_TAIL_SIZE:
                raise StartingWeaponTableError(
                    f"records[{index}].opaque_26_3f must contain 0x1A bytes"
                )
            output.extend(entry.opaque_tail)
        return bytes(output)

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "record_count": len(self.entries),
            "categories": list(self.categories),
            "records": [
                {
                    "name": entry.name,
                    "category": entry.category,
                    "material_id": entry.material_id,
                    "unknown_attribute_bits": entry.unknown_attribute_bits,
                    "unknown_words": {
                        "offset_18": entry.unknown_word_18,
                        "offset_1c": entry.unknown_word_1c,
                        "offset_20": entry.unknown_word_20,
                    },
                    "primary_value": entry.primary_value,
                    "opaque_26_3f": entry.opaque_tail.hex(),
                }
                for entry in self.entries
            ],
        }


def _decode_name(index: int, raw: bytes) -> str:
    end = raw.find(b"\0")
    if end < 0:
        end = len(raw)
    elif any(raw[end:]):
        raise StartingWeaponTableError(
            f"record {index} name contains nonzero bytes after its terminator"
        )
    try:
        name = raw[:end].decode("ascii")
    except UnicodeDecodeError as error:
        raise StartingWeaponTableError(f"record {index} name is not ASCII") from error
    return _validate_name(index, name)


def _encode_name(index: int, value: object) -> bytes:
    name = _validate_name(index, value)
    try:
        encoded = name.encode("ascii")
    except UnicodeEncodeError as error:
        raise StartingWeaponTableError(f"records[{index}].name must be ASCII") from error
    if len(encoded) > NAME_SIZE:
        raise StartingWeaponTableError(
            f"records[{index}].name exceeds {NAME_SIZE} encoded bytes"
        )
    return encoded.ljust(NAME_SIZE, b"\0")


def _validate_name(index: int, value: object) -> str:
    if not isinstance(value, str) or not value:
        raise StartingWeaponTableError(f"records[{index}].name must be nonempty text")
    return value


def _validate_categories(values: tuple[object, ...]) -> tuple[str, ...]:
    categories = tuple(
        value
        if isinstance(value, str) and value
        else _raise_invalid_category_name(index)
        for index, value in enumerate(values)
    )
    if len(categories) > 0x40:
        raise StartingWeaponTableError("categories cannot contain more than 64 names")
    if len(set(categories)) != len(categories):
        raise StartingWeaponTableError("categories must contain unique names")
    return categories


def _raise_invalid_category_name(index: int):
    raise StartingWeaponTableError(f"categories[{index}] must be a nonempty string")


def _validate_category(index: int, value: object, categories: tuple[str, ...]) -> str:
    if not isinstance(value, str) or value not in categories:
        raise StartingWeaponTableError(
            f"records[{index}].category must name an entry from categories"
        )
    return value


def _validate_int(label: str, value: object, maximum: int) -> int:
    if type(value) is not int or not 0 <= value <= maximum:
        raise StartingWeaponTableError(f"{label} must be an integer in [0, {maximum}]")
    return value


def _validate_u32(label: str, value: object) -> int:
    return _validate_int(label, value, 0xFFFFFFFF)


def _parse_hex_bytes(label: str, value: object, expected_size: int) -> bytes:
    if not isinstance(value, str):
        raise StartingWeaponTableError(f"{label} must be a hexadecimal string")
    try:
        data = bytes.fromhex(value)
    except ValueError as error:
        raise StartingWeaponTableError(f"{label} is not valid hexadecimal") from error
    if len(data) != expected_size:
        raise StartingWeaponTableError(
            f"{label} must encode 0x{expected_size:X} bytes"
        )
    return data


def dump_starting_weapon_table_yaml(table: StartingWeaponTable) -> str:
    document = table.document()
    for record in document["records"]:
        record["unknown_attribute_bits"] = _HexInt(record["unknown_attribute_bits"])
        for key, value in record["unknown_words"].items():
            record["unknown_words"][key] = _HexInt(value)
    return yaml.dump(
        document,
        Dumper=_StartingWeaponDumper,
        sort_keys=False,
        allow_unicode=False,
        width=100,
    )


def load_starting_weapon_table_yaml(path: Path) -> StartingWeaponTable:
    try:
        document = yaml.safe_load(path.read_text(encoding="ascii"))
    except UnicodeDecodeError as error:
        raise StartingWeaponTableError(f"{path} must contain ASCII text") from error
    except yaml.YAMLError as error:
        raise StartingWeaponTableError(f"invalid YAML in {path}: {error}") from error
    return StartingWeaponTable.parse_document(document)


def validate_roundtrip(yaml_path: Path, binary_path: Path) -> StartingWeaponTable:
    table = load_starting_weapon_table_yaml(yaml_path)
    if table.to_bytes() != binary_path.read_bytes():
        raise StartingWeaponTableError(f"{yaml_path} rebuild differs from {binary_path}")
    return table


def _summary(path: Path, table: StartingWeaponTable) -> str:
    return f"{path}: {len(table.entries)} records, 0x{len(table.to_bytes()):X} bytes"


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
            table = load_starting_weapon_table_yaml(args.source)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(table.to_bytes())
        elif args.command == "validate":
            for path in args.paths:
                table = load_starting_weapon_table_yaml(path)
                print(f"{_summary(path, table)} [valid]")
        elif args.command == "roundtrip":
            table = validate_roundtrip(args.source, args.binary)
            print(f"{_summary(args.source, table)} [round-trip OK]")
    except (OSError, StartingWeaponTableError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
