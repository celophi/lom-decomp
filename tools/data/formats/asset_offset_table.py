#!/usr/bin/env python3
"""Extract and rebuild counted tables of self-relative asset offsets."""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml


FORMAT_NAME = "self_relative_asset_offsets"
FORMAT_VERSION = 1
WORD_SIZE = 4


class _HexInt(int):
    pass


class _AssetOffsetDumper(yaml.SafeDumper):
    pass


_AssetOffsetDumper.add_representer(
    _HexInt,
    lambda dumper, value: dumper.represent_scalar(
        "tag:yaml.org,2002:int", f"0x{int(value):X}"
    ),
)


class AssetOffsetTableError(ValueError):
    """Raised when an asset-offset table or its YAML document is invalid."""


@dataclass(frozen=True)
class AssetOffsetEntry:
    target: str
    offset: int


@dataclass(frozen=True)
class AssetOffsetTable:
    """A count followed by self-relative u32 offsets to named assets."""

    entries: tuple[AssetOffsetEntry, ...]

    @classmethod
    def parse_binary(
        cls, data: bytes, target_names: tuple[str, ...]
    ) -> "AssetOffsetTable":
        if len(data) < WORD_SIZE or len(data) % WORD_SIZE != 0:
            raise AssetOffsetTableError(
                f"binary size 0x{len(data):X} is not a nonempty multiple of 4"
            )

        count = struct.unpack_from("<I", data)[0]
        actual_count = len(data) // WORD_SIZE - 1
        if count != actual_count:
            raise AssetOffsetTableError(
                f"header declares {count} entries, but binary contains {actual_count}"
            )
        if len(target_names) != count:
            raise AssetOffsetTableError(
                f"received {len(target_names)} target names for {count} entries"
            )

        offsets = struct.unpack_from(f"<{count}I", data, WORD_SIZE) if count else ()
        entries = tuple(
            AssetOffsetEntry(_validate_target(index, target), offset)
            for index, (target, offset) in enumerate(zip(target_names, offsets))
        )
        _validate_offsets(entries)
        return cls(entries)

    @classmethod
    def parse_document(cls, document: object) -> "AssetOffsetTable":
        if not isinstance(document, dict):
            raise AssetOffsetTableError("YAML root must be a mapping")

        expected_keys = {"format", "version", "entry_count", "entries"}
        actual_keys = set(document)
        if actual_keys != expected_keys:
            missing = sorted(expected_keys - actual_keys)
            unknown = sorted(actual_keys - expected_keys)
            details = []
            if missing:
                details.append(f"missing keys: {', '.join(missing)}")
            if unknown:
                details.append(f"unknown keys: {', '.join(unknown)}")
            raise AssetOffsetTableError(
                "invalid YAML schema (" + "; ".join(details) + ")"
            )
        if document["format"] != FORMAT_NAME:
            raise AssetOffsetTableError(f"format must be '{FORMAT_NAME}'")
        if document["version"] != FORMAT_VERSION:
            raise AssetOffsetTableError(f"version must be {FORMAT_VERSION}")

        entry_count = _validate_int("entry_count", document["entry_count"])
        raw_entries = document["entries"]
        if not isinstance(raw_entries, list):
            raise AssetOffsetTableError("entries must be a sequence")
        if len(raw_entries) != entry_count:
            raise AssetOffsetTableError(
                f"YAML contains {len(raw_entries)} entries, but entry_count is {entry_count}"
            )

        entries = []
        for index, raw_entry in enumerate(raw_entries):
            if not isinstance(raw_entry, dict) or set(raw_entry) != {"target", "offset"}:
                raise AssetOffsetTableError(
                    f"entries[{index}] must contain exactly target and offset"
                )
            entries.append(
                AssetOffsetEntry(
                    _validate_target(index, raw_entry["target"]),
                    _validate_int(f"entries[{index}].offset", raw_entry["offset"]),
                )
            )

        result = cls(tuple(entries))
        _validate_offsets(result.entries)
        return result

    def to_bytes(self) -> bytes:
        _validate_offsets(self.entries)
        output = bytearray(struct.pack("<I", len(self.entries)))
        for index, entry in enumerate(self.entries):
            output.extend(
                struct.pack("<I", _validate_int(f"entries[{index}].offset", entry.offset))
            )
        return bytes(output)

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "entry_count": len(self.entries),
            "entries": [
                {"target": entry.target, "offset": entry.offset}
                for entry in self.entries
            ],
        }


def _validate_target(index: int, value: object) -> str:
    if not isinstance(value, str) or not value:
        raise AssetOffsetTableError(f"entries[{index}].target must be a nonempty string")
    return value


def _validate_int(label: str, value: object) -> int:
    if type(value) is not int or not 0 <= value <= 0xFFFFFFFF:
        raise AssetOffsetTableError(f"{label} must be a u32 integer")
    return value


def _validate_offsets(entries: tuple[AssetOffsetEntry, ...]) -> None:
    table_size = (len(entries) + 1) * WORD_SIZE
    previous = None
    targets = set()
    for index, entry in enumerate(entries):
        if entry.target in targets:
            raise AssetOffsetTableError(f"duplicate target {entry.target!r}")
        targets.add(entry.target)
        offset = _validate_int(f"entries[{index}].offset", entry.offset)
        if offset < table_size:
            raise AssetOffsetTableError(
                f"entries[{index}].offset 0x{offset:X} points inside the table"
            )
        if previous is not None and offset <= previous:
            raise AssetOffsetTableError("asset offsets must be strictly increasing")
        previous = offset


def dump_asset_offset_table_yaml(table: AssetOffsetTable) -> str:
    document = table.document()
    for entry in document["entries"]:
        entry["offset"] = _HexInt(entry["offset"])
    return yaml.dump(
        document,
        Dumper=_AssetOffsetDumper,
        sort_keys=False,
        allow_unicode=False,
        width=100,
    )


def load_asset_offset_table_yaml(path: Path) -> AssetOffsetTable:
    try:
        document = yaml.safe_load(path.read_text(encoding="ascii"))
    except UnicodeDecodeError as error:
        raise AssetOffsetTableError(f"{path} must contain ASCII text") from error
    except yaml.YAMLError as error:
        raise AssetOffsetTableError(f"invalid YAML in {path}: {error}") from error
    return AssetOffsetTable.parse_document(document)


def validate_roundtrip(yaml_path: Path, binary_path: Path) -> AssetOffsetTable:
    table = load_asset_offset_table_yaml(yaml_path)
    if table.to_bytes() != binary_path.read_bytes():
        raise AssetOffsetTableError(f"{yaml_path} rebuild differs from {binary_path}")
    return table


def _summary(path: Path, table: AssetOffsetTable) -> str:
    return f"{path}: {len(table.entries)} assets, 0x{len(table.to_bytes()):X} bytes"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    extract_parser = subparsers.add_parser("extract")
    extract_parser.add_argument("source", type=Path)
    extract_parser.add_argument("output", type=Path)
    extract_parser.add_argument("--target", action="append", default=[])

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
        if args.command == "extract":
            table = AssetOffsetTable.parse_binary(
                args.source.read_bytes(), tuple(args.target)
            )
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(
                dump_asset_offset_table_yaml(table), encoding="ascii", newline="\n"
            )
        elif args.command == "build":
            table = load_asset_offset_table_yaml(args.source)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(table.to_bytes())
        elif args.command == "validate":
            for path in args.paths:
                table = load_asset_offset_table_yaml(path)
                print(f"{_summary(path, table)} [valid]")
        elif args.command == "roundtrip":
            table = validate_roundtrip(args.source, args.binary)
            print(f"{_summary(args.source, table)} [round-trip OK]")
    except (OSError, AssetOffsetTableError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
