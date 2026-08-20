#!/usr/bin/env python3
"""Extract and rebuild PSX TIM upload descriptor tables."""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml


FORMAT_NAME = "psx_tim_upload_table"
FORMAT_VERSION = 1
RECORD_SIZE = 0x10


class _HexInt(int):
    pass


class _TimUploadDumper(yaml.SafeDumper):
    pass


_TimUploadDumper.add_representer(
    _HexInt,
    lambda dumper, value: dumper.represent_scalar(
        "tag:yaml.org,2002:int", f"0x{int(value):08X}"
    ),
)


class TimUploadTableError(ValueError):
    """Raised when a TIM upload table or its YAML document is invalid."""


@dataclass(frozen=True)
class TimUploadEntry:
    target: str
    texture_x: int
    texture_y: int
    clut_x: int
    clut_y: int
    source_address: int
    initial_control: int


@dataclass(frozen=True)
class TimUploadTable:
    entries: tuple[TimUploadEntry, ...]

    @classmethod
    def parse_binary(
        cls, data: bytes, target_names: tuple[str, ...]
    ) -> "TimUploadTable":
        if len(data) % RECORD_SIZE != 0:
            raise TimUploadTableError(
                f"binary size 0x{len(data):X} is not divisible by record size 0x10"
            )
        count = len(data) // RECORD_SIZE
        if len(target_names) != count:
            raise TimUploadTableError(
                f"received {len(target_names)} target names for {count} records"
            )

        entries = []
        for index, target in enumerate(target_names):
            values = struct.unpack_from("<hhhhII", data, index * RECORD_SIZE)
            entries.append(
                TimUploadEntry(
                    _validate_target(index, target),
                    values[0],
                    values[1],
                    values[2],
                    values[3],
                    values[4],
                    values[5],
                )
            )
        result = cls(tuple(entries))
        _validate_entries(result.entries)
        return result

    @classmethod
    def parse_document(cls, document: object) -> "TimUploadTable":
        if not isinstance(document, dict):
            raise TimUploadTableError("YAML root must be a mapping")
        expected_keys = {"format", "version", "entry_count", "entries"}
        if set(document) != expected_keys:
            raise TimUploadTableError(
                "YAML must contain format, version, entry_count, and entries"
            )
        if document["format"] != FORMAT_NAME:
            raise TimUploadTableError(f"format must be '{FORMAT_NAME}'")
        if document["version"] != FORMAT_VERSION:
            raise TimUploadTableError(f"version must be {FORMAT_VERSION}")
        count = _validate_u32("entry_count", document["entry_count"])
        raw_entries = document["entries"]
        if not isinstance(raw_entries, list):
            raise TimUploadTableError("entries must be a sequence")
        if len(raw_entries) != count:
            raise TimUploadTableError(
                f"YAML contains {len(raw_entries)} entries, but entry_count is {count}"
            )

        entries = []
        entry_keys = {
            "target",
            "texture_vram",
            "clut_vram",
            "source_address",
            "initial_control",
        }
        for index, raw_entry in enumerate(raw_entries):
            if not isinstance(raw_entry, dict) or set(raw_entry) != entry_keys:
                raise TimUploadTableError(f"entries[{index}] has an invalid schema")
            texture = _parse_coord(index, "texture_vram", raw_entry["texture_vram"])
            clut = _parse_coord(index, "clut_vram", raw_entry["clut_vram"])
            entries.append(
                TimUploadEntry(
                    _validate_target(index, raw_entry["target"]),
                    texture[0],
                    texture[1],
                    clut[0],
                    clut[1],
                    _validate_u32(
                        f"entries[{index}].source_address",
                        raw_entry["source_address"],
                    ),
                    _validate_u32(
                        f"entries[{index}].initial_control",
                        raw_entry["initial_control"],
                    ),
                )
            )
        result = cls(tuple(entries))
        _validate_entries(result.entries)
        return result

    def to_bytes(self) -> bytes:
        _validate_entries(self.entries)
        output = bytearray()
        for entry in self.entries:
            output.extend(
                struct.pack(
                    "<hhhhII",
                    entry.texture_x,
                    entry.texture_y,
                    entry.clut_x,
                    entry.clut_y,
                    entry.source_address,
                    entry.initial_control,
                )
            )
        return bytes(output)

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "entry_count": len(self.entries),
            "entries": [
                {
                    "target": entry.target,
                    "texture_vram": {"x": entry.texture_x, "y": entry.texture_y},
                    "clut_vram": {"x": entry.clut_x, "y": entry.clut_y},
                    "source_address": entry.source_address,
                    "initial_control": entry.initial_control,
                }
                for entry in self.entries
            ],
        }


def _validate_target(index: int, value: object) -> str:
    if not isinstance(value, str) or not value:
        raise TimUploadTableError(f"entries[{index}].target must be a nonempty string")
    return value


def _validate_s16(label: str, value: object) -> int:
    if type(value) is not int or not -0x8000 <= value <= 0x7FFF:
        raise TimUploadTableError(f"{label} must be a signed 16-bit integer")
    return value


def _validate_u32(label: str, value: object) -> int:
    if type(value) is not int or not 0 <= value <= 0xFFFFFFFF:
        raise TimUploadTableError(f"{label} must be a u32 integer")
    return value


def _parse_coord(index: int, label: str, value: object) -> tuple[int, int]:
    if not isinstance(value, dict) or set(value) != {"x", "y"}:
        raise TimUploadTableError(f"entries[{index}].{label} must contain x and y")
    return (
        _validate_s16(f"entries[{index}].{label}.x", value["x"]),
        _validate_s16(f"entries[{index}].{label}.y", value["y"]),
    )


def _validate_entries(entries: tuple[TimUploadEntry, ...]) -> None:
    targets = set()
    for index, entry in enumerate(entries):
        if entry.target in targets:
            raise TimUploadTableError(f"duplicate target {entry.target!r}")
        targets.add(entry.target)
        _validate_s16(f"entries[{index}].texture_x", entry.texture_x)
        _validate_s16(f"entries[{index}].texture_y", entry.texture_y)
        _validate_s16(f"entries[{index}].clut_x", entry.clut_x)
        _validate_s16(f"entries[{index}].clut_y", entry.clut_y)
        address = _validate_u32(
            f"entries[{index}].source_address", entry.source_address
        )
        if address & 3:
            raise TimUploadTableError(
                f"entries[{index}].source_address must be 4-byte aligned"
            )
        _validate_u32(f"entries[{index}].initial_control", entry.initial_control)


def dump_tim_upload_table_yaml(table: TimUploadTable) -> str:
    document = table.document()
    for entry in document["entries"]:
        entry["source_address"] = _HexInt(entry["source_address"])
        entry["initial_control"] = _HexInt(entry["initial_control"])
    return yaml.dump(
        document,
        Dumper=_TimUploadDumper,
        sort_keys=False,
        allow_unicode=False,
        width=100,
    )


def load_tim_upload_table_yaml(path: Path) -> TimUploadTable:
    try:
        document = yaml.safe_load(path.read_text(encoding="ascii"))
    except UnicodeDecodeError as error:
        raise TimUploadTableError(f"{path} must contain ASCII text") from error
    except yaml.YAMLError as error:
        raise TimUploadTableError(f"invalid YAML in {path}: {error}") from error
    return TimUploadTable.parse_document(document)


def validate_roundtrip(yaml_path: Path, binary_path: Path) -> TimUploadTable:
    table = load_tim_upload_table_yaml(yaml_path)
    if table.to_bytes() != binary_path.read_bytes():
        raise TimUploadTableError(f"{yaml_path} rebuild differs from {binary_path}")
    return table


def _summary(path: Path, table: TimUploadTable) -> str:
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
            table = load_tim_upload_table_yaml(args.source)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(table.to_bytes())
        elif args.command == "validate":
            for path in args.paths:
                table = load_tim_upload_table_yaml(path)
                print(f"{_summary(path, table)} [valid]")
        elif args.command == "roundtrip":
            table = validate_roundtrip(args.source, args.binary)
            print(f"{_summary(args.source, table)} [round-trip OK]")
    except (OSError, TimUploadTableError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
