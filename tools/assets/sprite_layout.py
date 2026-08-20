#!/usr/bin/env python3
"""Extract and rebuild fixed-record PSX sprite layout assets."""

from __future__ import annotations

import argparse
import json
import struct
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import yaml


FORMAT_NAME = "psx_sprite_layout"
FORMAT_VERSION = 1
RECORD_SIZE = 8


class SpriteLayoutError(ValueError):
    """Raised when a sprite layout binary or YAML document is invalid."""


@dataclass(frozen=True)
class SpriteLayoutEntry:
    """One glyph selection and signed screen position."""

    glyph_id: int
    x: int
    y: int

    def to_bytes(self) -> bytes:
        _validate_int("glyph_id", self.glyph_id, 0, 0xFFFFFFFF)
        _validate_int("x", self.x, -0x8000, 0x7FFF)
        _validate_int("y", self.y, -0x8000, 0x7FFF)
        return struct.pack("<Ihh", self.glyph_id, self.x, self.y)

    def document(self) -> dict[str, int]:
        return {"glyph_id": self.glyph_id, "x": self.x, "y": self.y}


@dataclass(frozen=True)
class SpriteLayout:
    """Ordered collection of fixed-size sprite placement records."""

    sprites: tuple[SpriteLayoutEntry, ...]

    @classmethod
    def parse_binary(
        cls, data: bytes, expected_count: int | None = None
    ) -> "SpriteLayout":
        if len(data) % RECORD_SIZE != 0:
            raise SpriteLayoutError(
                f"binary size 0x{len(data):X} is not divisible by record size "
                f"0x{RECORD_SIZE:X}"
            )

        count = len(data) // RECORD_SIZE
        if expected_count is not None and count != expected_count:
            raise SpriteLayoutError(
                f"binary contains {count} records; expected {expected_count}"
            )

        sprites = tuple(
            SpriteLayoutEntry(*struct.unpack_from("<Ihh", data, offset))
            for offset in range(0, len(data), RECORD_SIZE)
        )
        return cls(sprites)

    @classmethod
    def parse_document(
        cls, document: object, expected_count: int | None = None
    ) -> "SpriteLayout":
        if not isinstance(document, dict):
            raise SpriteLayoutError("YAML root must be a mapping")

        expected_keys = {"format", "version", "record_count", "sprites"}
        actual_keys = set(document)
        if actual_keys != expected_keys:
            missing = sorted(expected_keys - actual_keys)
            unknown = sorted(actual_keys - expected_keys)
            details = []
            if missing:
                details.append(f"missing keys: {', '.join(missing)}")
            if unknown:
                details.append(f"unknown keys: {', '.join(unknown)}")
            raise SpriteLayoutError("invalid YAML schema (" + "; ".join(details) + ")")

        if document["format"] != FORMAT_NAME:
            raise SpriteLayoutError(
                f"format must be '{FORMAT_NAME}', got {document['format']!r}"
            )
        if document["version"] != FORMAT_VERSION:
            raise SpriteLayoutError(
                f"version must be {FORMAT_VERSION}, got {document['version']!r}"
            )

        raw_sprites = document["sprites"]
        if not isinstance(raw_sprites, list):
            raise SpriteLayoutError("sprites must be a sequence")

        record_count = _validate_int(
            "record_count", document["record_count"], 0, 0xFFFFFFFF
        )
        if len(raw_sprites) != record_count:
            raise SpriteLayoutError(
                f"YAML contains {len(raw_sprites)} records, but record_count is "
                f"{record_count}"
            )

        sprites = []
        for index, raw_entry in enumerate(raw_sprites):
            if not isinstance(raw_entry, dict):
                raise SpriteLayoutError(f"sprites[{index}] must be a mapping")
            if set(raw_entry) != {"glyph_id", "x", "y"}:
                raise SpriteLayoutError(
                    f"sprites[{index}] must contain exactly glyph_id, x, and y"
                )

            glyph_id = _validate_int(
                f"sprites[{index}].glyph_id", raw_entry["glyph_id"], 0, 0xFFFFFFFF
            )
            x = _validate_int(f"sprites[{index}].x", raw_entry["x"], -0x8000, 0x7FFF)
            y = _validate_int(f"sprites[{index}].y", raw_entry["y"], -0x8000, 0x7FFF)
            sprites.append(SpriteLayoutEntry(glyph_id, x, y))

        if expected_count is not None and len(sprites) != expected_count:
            raise SpriteLayoutError(
                f"YAML contains {len(sprites)} records; expected {expected_count}"
            )

        return cls(tuple(sprites))

    def to_bytes(self) -> bytes:
        return b"".join(sprite.to_bytes() for sprite in self.sprites)

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "record_count": len(self.sprites),
            "sprites": [sprite.document() for sprite in self.sprites],
        }


def _validate_int(label: str, value: object, minimum: int, maximum: int) -> int:
    if type(value) is not int:
        raise SpriteLayoutError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise SpriteLayoutError(
            f"{label} value {value} is outside [{minimum}, {maximum}]"
        )
    return value


def parse_layout_binary(
    data: bytes, expected_count: int | None = None
) -> SpriteLayout:
    """Parse a complete fixed-record sprite layout binary."""
    return SpriteLayout.parse_binary(data, expected_count)


def dump_layout_yaml(layout: SpriteLayout) -> str:
    """Serialize a sprite layout into the canonical YAML representation."""
    return yaml.safe_dump(
        layout.document(), sort_keys=False, allow_unicode=False, width=100
    )


def load_layout_yaml(
    path: Path, expected_count: int | None = None
) -> SpriteLayout:
    """Read and validate one sprite layout YAML document."""
    try:
        document = yaml.safe_load(path.read_text(encoding="ascii"))
    except UnicodeDecodeError as error:
        raise SpriteLayoutError(f"{path} must contain ASCII text") from error
    except yaml.YAMLError as error:
        raise SpriteLayoutError(f"invalid YAML in {path}: {error}") from error
    return SpriteLayout.parse_document(document, expected_count)


def validate_roundtrip(
    yaml_path: Path, binary_path: Path, expected_count: int | None = None
) -> SpriteLayout:
    """Confirm that a YAML layout rebuilds to an existing binary exactly."""
    layout = load_layout_yaml(yaml_path, expected_count)
    rebuilt = layout.to_bytes()
    source = binary_path.read_bytes()
    if rebuilt != source:
        raise SpriteLayoutError(
            f"{yaml_path} rebuild differs from {binary_path}"
        )
    return layout


def _summary(path: Path, layout: SpriteLayout) -> str:
    return f"{path}: {len(layout.sprites)} sprites, 0x{len(layout.to_bytes()):X} bytes"


def _validate_paths(paths: Iterable[Path]) -> None:
    for path in paths:
        layout = load_layout_yaml(path)
        print(f"{_summary(path, layout)} [valid]")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    extract_parser = subparsers.add_parser(
        "extract", help="extract a binary layout to YAML"
    )
    extract_parser.add_argument("source", type=Path)
    extract_parser.add_argument("output", type=Path)
    extract_parser.add_argument("--expected-count", type=int)

    build_parser = subparsers.add_parser(
        "build", help="build a binary layout from YAML"
    )
    build_parser.add_argument("source", type=Path)
    build_parser.add_argument("output", type=Path)
    build_parser.add_argument("--expected-count", type=int)

    validate_parser = subparsers.add_parser(
        "validate", help="validate sprite layout YAML files"
    )
    validate_parser.add_argument("paths", nargs="+", type=Path)

    roundtrip_parser = subparsers.add_parser(
        "roundtrip", help="compare one YAML rebuild with its binary"
    )
    roundtrip_parser.add_argument("source", type=Path)
    roundtrip_parser.add_argument("binary", type=Path)
    roundtrip_parser.add_argument("--expected-count", type=int)

    info_parser = subparsers.add_parser("info", help="print layout metadata")
    info_parser.add_argument("path", type=Path)
    info_parser.add_argument("--json", action="store_true")

    args = parser.parse_args()

    try:
        if args.command == "extract":
            layout = parse_layout_binary(
                args.source.read_bytes(), args.expected_count
            )
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(
                dump_layout_yaml(layout), encoding="ascii", newline="\n"
            )
            print(f"Wrote {args.output}")
        elif args.command == "build":
            layout = load_layout_yaml(args.source, args.expected_count)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(layout.to_bytes())
            print(f"Wrote {args.output}")
        elif args.command == "validate":
            _validate_paths(args.paths)
        elif args.command == "roundtrip":
            layout = validate_roundtrip(
                args.source, args.binary, args.expected_count
            )
            print(f"{_summary(args.source, layout)} [round-trip OK]")
        elif args.command == "info":
            layout = load_layout_yaml(args.path)
            if args.json:
                print(json.dumps(layout.document(), indent=2))
            else:
                print(_summary(args.path, layout))
    except (OSError, SpriteLayoutError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
