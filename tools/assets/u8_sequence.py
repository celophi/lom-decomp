#!/usr/bin/env python3
"""Extract and rebuild ordered sequences of unsigned bytes."""

from __future__ import annotations

import argparse
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml


FORMAT_NAME = "u8_sequence"
FORMAT_VERSION = 1


class U8SequenceError(ValueError):
    """Raised when a u8 sequence or its YAML document is invalid."""


@dataclass(frozen=True)
class U8Sequence:
    values: tuple[int, ...]

    @classmethod
    def parse_binary(
        cls, data: bytes, expected_count: int | None = None
    ) -> "U8Sequence":
        if expected_count is not None and len(data) != expected_count:
            raise U8SequenceError(
                f"binary contains {len(data)} values; expected {expected_count}"
            )
        return cls(tuple(data))

    @classmethod
    def parse_document(
        cls, document: object, expected_count: int | None = None
    ) -> "U8Sequence":
        if not isinstance(document, dict):
            raise U8SequenceError("YAML root must be a mapping")
        expected_keys = {"format", "version", "value_count", "values"}
        if set(document) != expected_keys:
            raise U8SequenceError("YAML must contain format, version, value_count, and values")
        if document["format"] != FORMAT_NAME:
            raise U8SequenceError(f"format must be '{FORMAT_NAME}'")
        if document["version"] != FORMAT_VERSION:
            raise U8SequenceError(f"version must be {FORMAT_VERSION}")
        count = _validate_int("value_count", document["value_count"], 0xFFFFFFFF)
        raw_values = document["values"]
        if not isinstance(raw_values, list):
            raise U8SequenceError("values must be a sequence")
        if len(raw_values) != count:
            raise U8SequenceError(
                f"YAML contains {len(raw_values)} values, but value_count is {count}"
            )
        values = tuple(
            _validate_int(f"values[{index}]", value, 0xFF)
            for index, value in enumerate(raw_values)
        )
        if expected_count is not None and len(values) != expected_count:
            raise U8SequenceError(
                f"YAML contains {len(values)} values; expected {expected_count}"
            )
        return cls(values)

    def to_bytes(self) -> bytes:
        return bytes(
            _validate_int(f"values[{index}]", value, 0xFF)
            for index, value in enumerate(self.values)
        )

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "value_count": len(self.values),
            "values": list(self.values),
        }


def _validate_int(label: str, value: object, maximum: int) -> int:
    if type(value) is not int or not 0 <= value <= maximum:
        raise U8SequenceError(f"{label} must be an integer in [0, {maximum}]")
    return value


def dump_u8_sequence_yaml(sequence: U8Sequence) -> str:
    return yaml.safe_dump(
        sequence.document(), sort_keys=False, allow_unicode=False, width=100
    )


def load_u8_sequence_yaml(
    path: Path, expected_count: int | None = None
) -> U8Sequence:
    try:
        document = yaml.safe_load(path.read_text(encoding="ascii"))
    except UnicodeDecodeError as error:
        raise U8SequenceError(f"{path} must contain ASCII text") from error
    except yaml.YAMLError as error:
        raise U8SequenceError(f"invalid YAML in {path}: {error}") from error
    return U8Sequence.parse_document(document, expected_count)


def validate_roundtrip(yaml_path: Path, binary_path: Path) -> U8Sequence:
    sequence = load_u8_sequence_yaml(yaml_path)
    if sequence.to_bytes() != binary_path.read_bytes():
        raise U8SequenceError(f"{yaml_path} rebuild differs from {binary_path}")
    return sequence


def _summary(path: Path, sequence: U8Sequence) -> str:
    return f"{path}: {len(sequence.values)} values, 0x{len(sequence.values):X} bytes"


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
            sequence = load_u8_sequence_yaml(args.source)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(sequence.to_bytes())
        elif args.command == "validate":
            for path in args.paths:
                sequence = load_u8_sequence_yaml(path)
                print(f"{_summary(path, sequence)} [valid]")
        elif args.command == "roundtrip":
            sequence = validate_roundtrip(args.source, args.binary)
            print(f"{_summary(args.source, sequence)} [round-trip OK]")
    except (OSError, U8SequenceError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
