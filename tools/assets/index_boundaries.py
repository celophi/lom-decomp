#!/usr/bin/env python3
"""Extract and rebuild little-endian u32 index-boundary tables."""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml


FORMAT_NAME = "u32_index_boundaries"
FORMAT_VERSION = 1
RECORD_SIZE = 4


class IndexBoundariesError(ValueError):
    """Raised when an index-boundary binary or YAML document is invalid."""


@dataclass(frozen=True)
class IndexBoundaries:
    """Nondecreasing boundaries delimiting adjacent indexed ranges."""

    boundaries: tuple[int, ...]

    @classmethod
    def parse_binary(
        cls, data: bytes, expected_count: int | None = None
    ) -> "IndexBoundaries":
        if len(data) % RECORD_SIZE != 0:
            raise IndexBoundariesError(
                f"binary size 0x{len(data):X} is not divisible by record size "
                f"0x{RECORD_SIZE:X}"
            )

        count = len(data) // RECORD_SIZE
        if expected_count is not None and count != expected_count:
            raise IndexBoundariesError(
                f"binary contains {count} boundaries; expected {expected_count}"
            )

        boundaries = tuple(
            struct.unpack_from("<I", data, offset)[0]
            for offset in range(0, len(data), RECORD_SIZE)
        )
        _validate_order(boundaries)
        return cls(boundaries)

    @classmethod
    def parse_document(
        cls, document: object, expected_count: int | None = None
    ) -> "IndexBoundaries":
        if not isinstance(document, dict):
            raise IndexBoundariesError("YAML root must be a mapping")

        expected_keys = {"format", "version", "boundary_count", "boundaries"}
        actual_keys = set(document)
        if actual_keys != expected_keys:
            missing = sorted(expected_keys - actual_keys)
            unknown = sorted(actual_keys - expected_keys)
            details = []
            if missing:
                details.append(f"missing keys: {', '.join(missing)}")
            if unknown:
                details.append(f"unknown keys: {', '.join(unknown)}")
            raise IndexBoundariesError(
                "invalid YAML schema (" + "; ".join(details) + ")"
            )

        if document["format"] != FORMAT_NAME:
            raise IndexBoundariesError(
                f"format must be '{FORMAT_NAME}', got {document['format']!r}"
            )
        if document["version"] != FORMAT_VERSION:
            raise IndexBoundariesError(
                f"version must be {FORMAT_VERSION}, got {document['version']!r}"
            )

        boundary_count = _validate_int(
            "boundary_count", document["boundary_count"], 0, 0xFFFFFFFF
        )
        raw_boundaries = document["boundaries"]
        if not isinstance(raw_boundaries, list):
            raise IndexBoundariesError("boundaries must be a sequence")
        if len(raw_boundaries) != boundary_count:
            raise IndexBoundariesError(
                f"YAML contains {len(raw_boundaries)} boundaries, but "
                f"boundary_count is {boundary_count}"
            )

        boundaries = tuple(
            _validate_int(f"boundaries[{index}]", value, 0, 0xFFFFFFFF)
            for index, value in enumerate(raw_boundaries)
        )
        _validate_order(boundaries)

        if expected_count is not None and len(boundaries) != expected_count:
            raise IndexBoundariesError(
                f"YAML contains {len(boundaries)} boundaries; expected "
                f"{expected_count}"
            )

        return cls(boundaries)

    def to_bytes(self) -> bytes:
        _validate_order(self.boundaries)
        return b"".join(
            struct.pack(
                "<I", _validate_int(f"boundaries[{index}]", value, 0, 0xFFFFFFFF)
            )
            for index, value in enumerate(self.boundaries)
        )

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "boundary_count": len(self.boundaries),
            "boundaries": list(self.boundaries),
        }


def _validate_int(label: str, value: object, minimum: int, maximum: int) -> int:
    if type(value) is not int:
        raise IndexBoundariesError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise IndexBoundariesError(
            f"{label} value {value} is outside [{minimum}, {maximum}]"
        )
    return value


def _validate_order(boundaries: tuple[int, ...]) -> None:
    for index in range(1, len(boundaries)):
        if boundaries[index] < boundaries[index - 1]:
            raise IndexBoundariesError(
                f"boundaries[{index}] value {boundaries[index]} is less than "
                f"the preceding boundary {boundaries[index - 1]}"
            )


def dump_boundaries_yaml(boundaries: IndexBoundaries) -> str:
    """Serialize index boundaries into their canonical YAML representation."""
    return yaml.safe_dump(
        boundaries.document(), sort_keys=False, allow_unicode=False, width=100
    )


def load_boundaries_yaml(
    path: Path, expected_count: int | None = None
) -> IndexBoundaries:
    """Read and validate one index-boundaries YAML document."""
    try:
        document = yaml.safe_load(path.read_text(encoding="ascii"))
    except UnicodeDecodeError as error:
        raise IndexBoundariesError(f"{path} must contain ASCII text") from error
    except yaml.YAMLError as error:
        raise IndexBoundariesError(f"invalid YAML in {path}: {error}") from error
    return IndexBoundaries.parse_document(document, expected_count)


def validate_roundtrip(
    yaml_path: Path, binary_path: Path, expected_count: int | None = None
) -> IndexBoundaries:
    """Confirm that boundary YAML rebuilds to an existing binary exactly."""
    boundaries = load_boundaries_yaml(yaml_path, expected_count)
    if boundaries.to_bytes() != binary_path.read_bytes():
        raise IndexBoundariesError(f"{yaml_path} rebuild differs from {binary_path}")
    return boundaries


def _summary(path: Path, boundaries: IndexBoundaries) -> str:
    return (
        f"{path}: {len(boundaries.boundaries)} boundaries, "
        f"0x{len(boundaries.to_bytes()):X} bytes"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    extract_parser = subparsers.add_parser(
        "extract", help="extract binary index boundaries to YAML"
    )
    extract_parser.add_argument("source", type=Path)
    extract_parser.add_argument("output", type=Path)
    extract_parser.add_argument("--expected-count", type=int)

    build_parser = subparsers.add_parser(
        "build", help="build binary index boundaries from YAML"
    )
    build_parser.add_argument("source", type=Path)
    build_parser.add_argument("output", type=Path)
    build_parser.add_argument("--expected-count", type=int)

    validate_parser = subparsers.add_parser(
        "validate", help="validate index-boundary YAML files"
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
            boundaries = IndexBoundaries.parse_binary(
                args.source.read_bytes(), args.expected_count
            )
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(
                dump_boundaries_yaml(boundaries), encoding="ascii", newline="\n"
            )
            print(f"Wrote {args.output}")
        elif args.command == "build":
            boundaries = load_boundaries_yaml(args.source, args.expected_count)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(boundaries.to_bytes())
            print(f"Wrote {args.output}")
        elif args.command == "validate":
            for path in args.paths:
                boundaries = load_boundaries_yaml(path)
                print(f"{_summary(path, boundaries)} [valid]")
        elif args.command == "roundtrip":
            boundaries = validate_roundtrip(
                args.source, args.binary, args.expected_count
            )
            print(f"{_summary(args.source, boundaries)} [round-trip OK]")
    except (OSError, IndexBoundariesError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
