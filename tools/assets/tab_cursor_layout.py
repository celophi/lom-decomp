#!/usr/bin/env python3
"""Extract and rebuild packed PSX tab-cursor layout assets."""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml


FORMAT_NAME = "psx_tab_cursor_layout"
FORMAT_VERSION = 1
RECORD_SIZE = 4
X_MASK = 0x1FF
SPRITE_INDEX_MASK = 0x7F
SPRITE_INDEX_SHIFT = 9


class TabCursorLayoutError(ValueError):
    """Raised when a tab-cursor layout binary or YAML document is invalid."""


@dataclass(frozen=True)
class TabCursorEntry:
    """One packed cursor target and glyph selection."""

    x: int
    sprite_index: int
    y: int
    glyph_id: int

    def to_bytes(self) -> bytes:
        x = _validate_int("x", self.x, 0, X_MASK)
        sprite_index = _validate_int(
            "sprite_index", self.sprite_index, 0, SPRITE_INDEX_MASK
        )
        y = _validate_int("y", self.y, 0, 0xFF)
        glyph_id = _validate_int("glyph_id", self.glyph_id, 0, 0xFF)
        packed = x | (sprite_index << SPRITE_INDEX_SHIFT)
        return struct.pack("<HBB", packed, y, glyph_id)

    def document(self) -> dict[str, int]:
        return {
            "x": self.x,
            "sprite_index": self.sprite_index,
            "y": self.y,
            "glyph_id": self.glyph_id,
        }


@dataclass(frozen=True)
class TabCursorLayout:
    """Ordered collection of packed tab-cursor records."""

    entries: tuple[TabCursorEntry, ...]

    @classmethod
    def parse_binary(
        cls, data: bytes, expected_count: int | None = None
    ) -> "TabCursorLayout":
        if len(data) % RECORD_SIZE != 0:
            raise TabCursorLayoutError(
                f"binary size 0x{len(data):X} is not divisible by record size "
                f"0x{RECORD_SIZE:X}"
            )

        count = len(data) // RECORD_SIZE
        if expected_count is not None and count != expected_count:
            raise TabCursorLayoutError(
                f"binary contains {count} records; expected {expected_count}"
            )

        entries = []
        for offset in range(0, len(data), RECORD_SIZE):
            packed, y, glyph_id = struct.unpack_from("<HBB", data, offset)
            entries.append(
                TabCursorEntry(
                    packed & X_MASK,
                    (packed >> SPRITE_INDEX_SHIFT) & SPRITE_INDEX_MASK,
                    y,
                    glyph_id,
                )
            )
        return cls(tuple(entries))

    @classmethod
    def parse_document(
        cls, document: object, expected_count: int | None = None
    ) -> "TabCursorLayout":
        if not isinstance(document, dict):
            raise TabCursorLayoutError("YAML root must be a mapping")

        expected_keys = {"format", "version", "record_count", "entries"}
        actual_keys = set(document)
        if actual_keys != expected_keys:
            missing = sorted(expected_keys - actual_keys)
            unknown = sorted(actual_keys - expected_keys)
            details = []
            if missing:
                details.append(f"missing keys: {', '.join(missing)}")
            if unknown:
                details.append(f"unknown keys: {', '.join(unknown)}")
            raise TabCursorLayoutError(
                "invalid YAML schema (" + "; ".join(details) + ")"
            )

        if document["format"] != FORMAT_NAME:
            raise TabCursorLayoutError(
                f"format must be '{FORMAT_NAME}', got {document['format']!r}"
            )
        if document["version"] != FORMAT_VERSION:
            raise TabCursorLayoutError(
                f"version must be {FORMAT_VERSION}, got {document['version']!r}"
            )

        record_count = _validate_int(
            "record_count", document["record_count"], 0, 0xFFFFFFFF
        )
        raw_entries = document["entries"]
        if not isinstance(raw_entries, list):
            raise TabCursorLayoutError("entries must be a sequence")
        if len(raw_entries) != record_count:
            raise TabCursorLayoutError(
                f"YAML contains {len(raw_entries)} records, but record_count is "
                f"{record_count}"
            )

        entries = []
        for index, raw_entry in enumerate(raw_entries):
            label = f"entries[{index}]"
            if not isinstance(raw_entry, dict):
                raise TabCursorLayoutError(f"{label} must be a mapping")
            if set(raw_entry) != {"x", "sprite_index", "y", "glyph_id"}:
                raise TabCursorLayoutError(
                    f"{label} must contain exactly x, sprite_index, y, and glyph_id"
                )
            entries.append(
                TabCursorEntry(
                    _validate_int(f"{label}.x", raw_entry["x"], 0, X_MASK),
                    _validate_int(
                        f"{label}.sprite_index",
                        raw_entry["sprite_index"],
                        0,
                        SPRITE_INDEX_MASK,
                    ),
                    _validate_int(f"{label}.y", raw_entry["y"], 0, 0xFF),
                    _validate_int(
                        f"{label}.glyph_id", raw_entry["glyph_id"], 0, 0xFF
                    ),
                )
            )

        if expected_count is not None and len(entries) != expected_count:
            raise TabCursorLayoutError(
                f"YAML contains {len(entries)} records; expected {expected_count}"
            )

        return cls(tuple(entries))

    def to_bytes(self) -> bytes:
        return b"".join(entry.to_bytes() for entry in self.entries)

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "record_count": len(self.entries),
            "entries": [entry.document() for entry in self.entries],
        }


def _validate_int(label: str, value: object, minimum: int, maximum: int) -> int:
    if type(value) is not int:
        raise TabCursorLayoutError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise TabCursorLayoutError(
            f"{label} value {value} is outside [{minimum}, {maximum}]"
        )
    return value


def dump_layout_yaml(layout: TabCursorLayout) -> str:
    """Serialize a tab-cursor layout into its canonical YAML representation."""
    return yaml.safe_dump(
        layout.document(), sort_keys=False, allow_unicode=False, width=100
    )


def load_layout_yaml(
    path: Path, expected_count: int | None = None
) -> TabCursorLayout:
    """Read and validate one tab-cursor layout YAML document."""
    try:
        document = yaml.safe_load(path.read_text(encoding="ascii"))
    except UnicodeDecodeError as error:
        raise TabCursorLayoutError(f"{path} must contain ASCII text") from error
    except yaml.YAMLError as error:
        raise TabCursorLayoutError(f"invalid YAML in {path}: {error}") from error
    return TabCursorLayout.parse_document(document, expected_count)


def validate_roundtrip(
    yaml_path: Path, binary_path: Path, expected_count: int | None = None
) -> TabCursorLayout:
    """Confirm that a tab-cursor YAML rebuilds to an existing binary exactly."""
    layout = load_layout_yaml(yaml_path, expected_count)
    if layout.to_bytes() != binary_path.read_bytes():
        raise TabCursorLayoutError(f"{yaml_path} rebuild differs from {binary_path}")
    return layout


def _summary(path: Path, layout: TabCursorLayout) -> str:
    return f"{path}: {len(layout.entries)} entries, 0x{len(layout.to_bytes()):X} bytes"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    extract_parser = subparsers.add_parser(
        "extract", help="extract a binary tab-cursor layout to YAML"
    )
    extract_parser.add_argument("source", type=Path)
    extract_parser.add_argument("output", type=Path)
    extract_parser.add_argument("--expected-count", type=int)

    build_parser = subparsers.add_parser(
        "build", help="build a binary tab-cursor layout from YAML"
    )
    build_parser.add_argument("source", type=Path)
    build_parser.add_argument("output", type=Path)
    build_parser.add_argument("--expected-count", type=int)

    validate_parser = subparsers.add_parser(
        "validate", help="validate tab-cursor layout YAML files"
    )
    validate_parser.add_argument("paths", nargs="+", type=Path)

    roundtrip_parser = subparsers.add_parser(
        "roundtrip", help="compare one YAML rebuild with its binary"
    )
    roundtrip_parser.add_argument("source", type=Path)
    roundtrip_parser.add_argument("binary", type=Path)
    roundtrip_parser.add_argument("--expected-count", type=int)

    args = parser.parse_args()

    try:
        if args.command == "extract":
            layout = TabCursorLayout.parse_binary(
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
            for path in args.paths:
                layout = load_layout_yaml(path)
                print(f"{_summary(path, layout)} [valid]")
        elif args.command == "roundtrip":
            layout = validate_roundtrip(
                args.source, args.binary, args.expected_count
            )
            print(f"{_summary(args.source, layout)} [round-trip OK]")
    except (OSError, TabCursorLayoutError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
