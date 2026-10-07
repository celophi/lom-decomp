#!/usr/bin/env python3
"""Extract and rebuild little-endian u32 index maps with an unmapped sentinel."""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml


FORMAT_NAME = "u32_index_map"
FORMAT_VERSION = 1
RECORD_SIZE = 4


class IndexMapError(ValueError):
    """Raised when an index-map binary or YAML document is invalid."""


@dataclass(frozen=True)
class IndexMap:
    """Ordered mapped indices, with None representing an unmapped entry."""

    unmapped_value: int
    entries: tuple[int | None, ...]

    @classmethod
    def parse_binary(
        cls,
        data: bytes,
        unmapped_value: int,
        expected_count: int | None = None,
    ) -> "IndexMap":
        unmapped_value = _validate_int(
            "unmapped_value", unmapped_value, 0, 0xFFFFFFFF
        )
        if len(data) % RECORD_SIZE != 0:
            raise IndexMapError(
                f"binary size 0x{len(data):X} is not divisible by record size "
                f"0x{RECORD_SIZE:X}"
            )

        count = len(data) // RECORD_SIZE
        if expected_count is not None and count != expected_count:
            raise IndexMapError(
                f"binary contains {count} entries; expected {expected_count}"
            )

        entries = tuple(
            None if value == unmapped_value else value
            for (value,) in struct.iter_unpack("<I", data)
        )
        return cls(unmapped_value, entries)

    @classmethod
    def parse_document(
        cls, document: object, expected_count: int | None = None
    ) -> "IndexMap":
        if not isinstance(document, dict):
            raise IndexMapError("YAML root must be a mapping")

        expected_keys = {
            "format",
            "version",
            "entry_count",
            "unmapped_value",
            "entries",
        }
        actual_keys = set(document)
        if actual_keys != expected_keys:
            missing = sorted(expected_keys - actual_keys)
            unknown = sorted(actual_keys - expected_keys)
            details = []
            if missing:
                details.append(f"missing keys: {', '.join(missing)}")
            if unknown:
                details.append(f"unknown keys: {', '.join(unknown)}")
            raise IndexMapError("invalid YAML schema (" + "; ".join(details) + ")")

        if document["format"] != FORMAT_NAME:
            raise IndexMapError(
                f"format must be '{FORMAT_NAME}', got {document['format']!r}"
            )
        if document["version"] != FORMAT_VERSION:
            raise IndexMapError(
                f"version must be {FORMAT_VERSION}, got {document['version']!r}"
            )

        entry_count = _validate_int(
            "entry_count", document["entry_count"], 0, 0xFFFFFFFF
        )
        unmapped_value = _validate_int(
            "unmapped_value", document["unmapped_value"], 0, 0xFFFFFFFF
        )
        raw_entries = document["entries"]
        if not isinstance(raw_entries, list):
            raise IndexMapError("entries must be a sequence")
        if len(raw_entries) != entry_count:
            raise IndexMapError(
                f"YAML contains {len(raw_entries)} entries, but entry_count is "
                f"{entry_count}"
            )

        entries = []
        for index, value in enumerate(raw_entries):
            if value is None:
                entries.append(None)
                continue
            value = _validate_int(f"entries[{index}]", value, 0, 0xFFFFFFFF)
            if value == unmapped_value:
                raise IndexMapError(
                    f"entries[{index}] must use null for unmapped value "
                    f"{unmapped_value}"
                )
            entries.append(value)

        if expected_count is not None and len(entries) != expected_count:
            raise IndexMapError(
                f"YAML contains {len(entries)} entries; expected {expected_count}"
            )

        return cls(unmapped_value, tuple(entries))

    def to_bytes(self) -> bytes:
        unmapped_value = _validate_int(
            "unmapped_value", self.unmapped_value, 0, 0xFFFFFFFF
        )
        output = bytearray()
        for index, entry in enumerate(self.entries):
            value = (
                unmapped_value
                if entry is None
                else _validate_int(f"entries[{index}]", entry, 0, 0xFFFFFFFF)
            )
            if entry is not None and value == unmapped_value:
                raise IndexMapError(
                    f"entries[{index}] collides with unmapped value {unmapped_value}"
                )
            output.extend(struct.pack("<I", value))
        return bytes(output)

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "entry_count": len(self.entries),
            "unmapped_value": self.unmapped_value,
            "entries": list(self.entries),
        }


def _validate_int(label: str, value: object, minimum: int, maximum: int) -> int:
    if type(value) is not int:
        raise IndexMapError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise IndexMapError(
            f"{label} value {value} is outside [{minimum}, {maximum}]"
        )
    return value


def dump_index_map_yaml(index_map: IndexMap) -> str:
    """Serialize an index map into its canonical YAML representation."""
    return yaml.safe_dump(
        index_map.document(), sort_keys=False, allow_unicode=False, width=100
    )


def load_index_map_yaml(
    path: Path, expected_count: int | None = None
) -> IndexMap:
    """Read and validate one index-map YAML document."""
    try:
        document = yaml.safe_load(path.read_text(encoding="ascii"))
    except UnicodeDecodeError as error:
        raise IndexMapError(f"{path} must contain ASCII text") from error
    except yaml.YAMLError as error:
        raise IndexMapError(f"invalid YAML in {path}: {error}") from error
    return IndexMap.parse_document(document, expected_count)


def validate_roundtrip(
    yaml_path: Path, binary_path: Path, expected_count: int | None = None
) -> IndexMap:
    """Confirm that index-map YAML rebuilds to an existing binary exactly."""
    index_map = load_index_map_yaml(yaml_path, expected_count)
    if index_map.to_bytes() != binary_path.read_bytes():
        raise IndexMapError(f"{yaml_path} rebuild differs from {binary_path}")
    return index_map


def _summary(path: Path, index_map: IndexMap) -> str:
    unmapped = sum(entry is None for entry in index_map.entries)
    return (
        f"{path}: {len(index_map.entries)} entries, {unmapped} unmapped, "
        f"0x{len(index_map.to_bytes()):X} bytes"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    extract_parser = subparsers.add_parser(
        "extract", help="extract a binary index map to YAML"
    )
    extract_parser.add_argument("source", type=Path)
    extract_parser.add_argument("output", type=Path)
    extract_parser.add_argument("--unmapped-value", type=int, required=True)
    extract_parser.add_argument("--expected-count", type=int)

    build_parser = subparsers.add_parser(
        "build", help="build a binary index map from YAML"
    )
    build_parser.add_argument("source", type=Path)
    build_parser.add_argument("output", type=Path)
    build_parser.add_argument("--expected-count", type=int)

    validate_parser = subparsers.add_parser(
        "validate", help="validate index-map YAML files"
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
            index_map = IndexMap.parse_binary(
                args.source.read_bytes(),
                args.unmapped_value,
                args.expected_count,
            )
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(
                dump_index_map_yaml(index_map), encoding="ascii", newline="\n"
            )
            print(f"Wrote {args.output}")
        elif args.command == "build":
            index_map = load_index_map_yaml(args.source, args.expected_count)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(index_map.to_bytes())
            print(f"Wrote {args.output}")
        elif args.command == "validate":
            for path in args.paths:
                index_map = load_index_map_yaml(path)
                print(f"{_summary(path, index_map)} [valid]")
        elif args.command == "roundtrip":
            index_map = validate_roundtrip(
                args.source, args.binary, args.expected_count
            )
            print(f"{_summary(args.source, index_map)} [round-trip OK]")
    except (OSError, IndexMapError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
