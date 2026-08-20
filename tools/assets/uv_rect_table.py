#!/usr/bin/env python3
"""Extract and rebuild compact six-byte UV rectangle tables."""

from __future__ import annotations

import argparse
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml


FORMAT_NAME = "psx_uv_rect_table"
FORMAT_VERSION = 1
RECORD_SIZE = 6
FIELD_NAMES = ("u", "v", "width", "height", "origin_x", "origin_y")


class UvRectTableError(ValueError):
    """Raised when a UV rectangle table or its YAML document is invalid."""


@dataclass(frozen=True)
class UvRect:
    u: int
    v: int
    width: int
    height: int
    origin_x: int
    origin_y: int


@dataclass(frozen=True)
class UvRectTable:
    unit_pixels: int
    entries: tuple[UvRect, ...]

    @classmethod
    def parse_binary(
        cls,
        data: bytes,
        unit_pixels: int,
        expected_count: int | None = None,
    ) -> "UvRectTable":
        unit_pixels = _validate_unit(unit_pixels)
        if len(data) % RECORD_SIZE != 0:
            raise UvRectTableError(
                f"binary size 0x{len(data):X} is not divisible by record size 6"
            )
        count = len(data) // RECORD_SIZE
        if expected_count is not None and count != expected_count:
            raise UvRectTableError(
                f"binary contains {count} rectangles; expected {expected_count}"
            )
        entries = tuple(
            UvRect(*(value * unit_pixels for value in data[offset : offset + 6]))
            for offset in range(0, len(data), RECORD_SIZE)
        )
        return cls(unit_pixels, entries)

    @classmethod
    def parse_document(
        cls, document: object, expected_count: int | None = None
    ) -> "UvRectTable":
        if not isinstance(document, dict):
            raise UvRectTableError("YAML root must be a mapping")
        expected_keys = {
            "format",
            "version",
            "unit_pixels",
            "entry_count",
            "entries",
        }
        if set(document) != expected_keys:
            raise UvRectTableError(
                "YAML must contain format, version, unit_pixels, entry_count, and entries"
            )
        if document["format"] != FORMAT_NAME:
            raise UvRectTableError(f"format must be '{FORMAT_NAME}'")
        if document["version"] != FORMAT_VERSION:
            raise UvRectTableError(f"version must be {FORMAT_VERSION}")
        unit_pixels = _validate_unit(document["unit_pixels"])
        count = _validate_int("entry_count", document["entry_count"], 0xFFFFFFFF)
        raw_entries = document["entries"]
        if not isinstance(raw_entries, list):
            raise UvRectTableError("entries must be a sequence")
        if len(raw_entries) != count:
            raise UvRectTableError(
                f"YAML contains {len(raw_entries)} entries, but entry_count is {count}"
            )
        if expected_count is not None and count != expected_count:
            raise UvRectTableError(
                f"YAML contains {count} rectangles; expected {expected_count}"
            )

        entries = []
        for index, raw_entry in enumerate(raw_entries):
            if not isinstance(raw_entry, dict) or set(raw_entry) != set(FIELD_NAMES):
                raise UvRectTableError(
                    f"entries[{index}] must contain fields: {', '.join(FIELD_NAMES)}"
                )
            values = tuple(
                _validate_pixel_value(
                    f"entries[{index}].{field}", raw_entry[field], unit_pixels
                )
                for field in FIELD_NAMES
            )
            entries.append(UvRect(*values))
        return cls(unit_pixels, tuple(entries))

    def to_bytes(self) -> bytes:
        unit_pixels = _validate_unit(self.unit_pixels)
        output = bytearray()
        for index, entry in enumerate(self.entries):
            for field in FIELD_NAMES:
                value = _validate_pixel_value(
                    f"entries[{index}].{field}", getattr(entry, field), unit_pixels
                )
                output.append(value // unit_pixels)
        return bytes(output)

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "unit_pixels": self.unit_pixels,
            "entry_count": len(self.entries),
            "entries": [
                {field: getattr(entry, field) for field in FIELD_NAMES}
                for entry in self.entries
            ],
        }


def _validate_int(label: str, value: object, maximum: int) -> int:
    if type(value) is not int or not 0 <= value <= maximum:
        raise UvRectTableError(f"{label} must be an integer in [0, {maximum}]")
    return value


def _validate_unit(value: object) -> int:
    value = _validate_int("unit_pixels", value, 0xFF)
    if value == 0:
        raise UvRectTableError("unit_pixels must be greater than zero")
    return value


def _validate_pixel_value(label: str, value: object, unit_pixels: int) -> int:
    value = _validate_int(label, value, unit_pixels * 0xFF)
    if value % unit_pixels != 0:
        raise UvRectTableError(f"{label} must be divisible by unit_pixels ({unit_pixels})")
    return value


def dump_uv_rect_table_yaml(table: UvRectTable) -> str:
    return yaml.safe_dump(
        table.document(), sort_keys=False, allow_unicode=False, width=100
    )


def load_uv_rect_table_yaml(path: Path) -> UvRectTable:
    try:
        document = yaml.safe_load(path.read_text(encoding="ascii"))
    except UnicodeDecodeError as error:
        raise UvRectTableError(f"{path} must contain ASCII text") from error
    except yaml.YAMLError as error:
        raise UvRectTableError(f"invalid YAML in {path}: {error}") from error
    return UvRectTable.parse_document(document)


def validate_roundtrip(yaml_path: Path, binary_path: Path) -> UvRectTable:
    table = load_uv_rect_table_yaml(yaml_path)
    if table.to_bytes() != binary_path.read_bytes():
        raise UvRectTableError(f"{yaml_path} rebuild differs from {binary_path}")
    return table


def _summary(path: Path, table: UvRectTable) -> str:
    return f"{path}: {len(table.entries)} rectangles, 0x{len(table.to_bytes()):X} bytes"


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
            table = load_uv_rect_table_yaml(args.source)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(table.to_bytes())
        elif args.command == "validate":
            for path in args.paths:
                table = load_uv_rect_table_yaml(path)
                print(f"{_summary(path, table)} [valid]")
        elif args.command == "roundtrip":
            table = validate_roundtrip(args.source, args.binary)
            print(f"{_summary(args.source, table)} [round-trip OK]")
    except (OSError, UvRectTableError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
