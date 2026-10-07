#!/usr/bin/env python3
"""Extract and rebuild fixed-record PSX glyph-metrics assets."""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml


FORMAT_NAME = "psx_glyph_metrics"
FORMAT_VERSION = 1
RECORD_SIZE = 8


class GlyphMetricsError(ValueError):
    """Raised when a glyph-metrics binary or YAML document is invalid."""


@dataclass(frozen=True)
class GlyphMetric:
    """Texture coordinates, dimensions, and CLUT selector for one glyph."""

    u: int
    v: int
    width: int
    height: int
    clut: int

    def to_bytes(self) -> bytes:
        u = _validate_int("u", self.u, 0, 0xFF)
        v = _validate_int("v", self.v, 0, 0xFF)
        width = _validate_int("width", self.width, 0, 0xFF)
        height = _validate_int("height", self.height, 0, 0xFF)
        clut = _validate_int("clut", self.clut, 0, 0xFFFFFFFF)
        return struct.pack("<BBBBI", u, v, width, height, clut)

    def document(self) -> dict[str, int]:
        return {
            "u": self.u,
            "v": self.v,
            "width": self.width,
            "height": self.height,
            "clut": self.clut,
        }


@dataclass(frozen=True)
class GlyphMetrics:
    """Ordered collection of fixed-size glyph-metric records."""

    glyphs: tuple[GlyphMetric, ...]

    @classmethod
    def parse_binary(
        cls, data: bytes, expected_count: int | None = None
    ) -> "GlyphMetrics":
        if len(data) % RECORD_SIZE != 0:
            raise GlyphMetricsError(
                f"binary size 0x{len(data):X} is not divisible by record size "
                f"0x{RECORD_SIZE:X}"
            )

        count = len(data) // RECORD_SIZE
        if expected_count is not None and count != expected_count:
            raise GlyphMetricsError(
                f"binary contains {count} records; expected {expected_count}"
            )

        glyphs = tuple(
            GlyphMetric(*struct.unpack_from("<BBBBI", data, offset))
            for offset in range(0, len(data), RECORD_SIZE)
        )
        return cls(glyphs)

    @classmethod
    def parse_document(
        cls, document: object, expected_count: int | None = None
    ) -> "GlyphMetrics":
        if not isinstance(document, dict):
            raise GlyphMetricsError("YAML root must be a mapping")

        expected_keys = {"format", "version", "record_count", "glyphs"}
        actual_keys = set(document)
        if actual_keys != expected_keys:
            missing = sorted(expected_keys - actual_keys)
            unknown = sorted(actual_keys - expected_keys)
            details = []
            if missing:
                details.append(f"missing keys: {', '.join(missing)}")
            if unknown:
                details.append(f"unknown keys: {', '.join(unknown)}")
            raise GlyphMetricsError(
                "invalid YAML schema (" + "; ".join(details) + ")"
            )

        if document["format"] != FORMAT_NAME:
            raise GlyphMetricsError(
                f"format must be '{FORMAT_NAME}', got {document['format']!r}"
            )
        if document["version"] != FORMAT_VERSION:
            raise GlyphMetricsError(
                f"version must be {FORMAT_VERSION}, got {document['version']!r}"
            )

        record_count = _validate_int(
            "record_count", document["record_count"], 0, 0xFFFFFFFF
        )
        raw_glyphs = document["glyphs"]
        if not isinstance(raw_glyphs, list):
            raise GlyphMetricsError("glyphs must be a sequence")
        if len(raw_glyphs) != record_count:
            raise GlyphMetricsError(
                f"YAML contains {len(raw_glyphs)} records, but record_count is "
                f"{record_count}"
            )

        glyphs = []
        for index, raw_glyph in enumerate(raw_glyphs):
            label = f"glyphs[{index}]"
            if not isinstance(raw_glyph, dict):
                raise GlyphMetricsError(f"{label} must be a mapping")
            if set(raw_glyph) != {"u", "v", "width", "height", "clut"}:
                raise GlyphMetricsError(
                    f"{label} must contain exactly u, v, width, height, and clut"
                )
            glyphs.append(
                GlyphMetric(
                    _validate_int(f"{label}.u", raw_glyph["u"], 0, 0xFF),
                    _validate_int(f"{label}.v", raw_glyph["v"], 0, 0xFF),
                    _validate_int(
                        f"{label}.width", raw_glyph["width"], 0, 0xFF
                    ),
                    _validate_int(
                        f"{label}.height", raw_glyph["height"], 0, 0xFF
                    ),
                    _validate_int(
                        f"{label}.clut", raw_glyph["clut"], 0, 0xFFFFFFFF
                    ),
                )
            )

        if expected_count is not None and len(glyphs) != expected_count:
            raise GlyphMetricsError(
                f"YAML contains {len(glyphs)} records; expected {expected_count}"
            )

        return cls(tuple(glyphs))

    def to_bytes(self) -> bytes:
        return b"".join(glyph.to_bytes() for glyph in self.glyphs)

    def document(self) -> dict[str, object]:
        return {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "record_count": len(self.glyphs),
            "glyphs": [glyph.document() for glyph in self.glyphs],
        }


def _validate_int(label: str, value: object, minimum: int, maximum: int) -> int:
    if type(value) is not int:
        raise GlyphMetricsError(f"{label} must be an integer")
    if not minimum <= value <= maximum:
        raise GlyphMetricsError(
            f"{label} value {value} is outside [{minimum}, {maximum}]"
        )
    return value


def dump_metrics_yaml(metrics: GlyphMetrics) -> str:
    """Serialize glyph metrics into their canonical YAML representation."""
    return yaml.safe_dump(
        metrics.document(), sort_keys=False, allow_unicode=False, width=100
    )


def load_metrics_yaml(
    path: Path, expected_count: int | None = None
) -> GlyphMetrics:
    """Read and validate one glyph-metrics YAML document."""
    try:
        document = yaml.safe_load(path.read_text(encoding="ascii"))
    except UnicodeDecodeError as error:
        raise GlyphMetricsError(f"{path} must contain ASCII text") from error
    except yaml.YAMLError as error:
        raise GlyphMetricsError(f"invalid YAML in {path}: {error}") from error
    return GlyphMetrics.parse_document(document, expected_count)


def validate_roundtrip(
    yaml_path: Path, binary_path: Path, expected_count: int | None = None
) -> GlyphMetrics:
    """Confirm that glyph-metrics YAML rebuilds to an existing binary exactly."""
    metrics = load_metrics_yaml(yaml_path, expected_count)
    if metrics.to_bytes() != binary_path.read_bytes():
        raise GlyphMetricsError(f"{yaml_path} rebuild differs from {binary_path}")
    return metrics


def _summary(path: Path, metrics: GlyphMetrics) -> str:
    return f"{path}: {len(metrics.glyphs)} glyphs, 0x{len(metrics.to_bytes()):X} bytes"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    extract_parser = subparsers.add_parser(
        "extract", help="extract binary glyph metrics to YAML"
    )
    extract_parser.add_argument("source", type=Path)
    extract_parser.add_argument("output", type=Path)
    extract_parser.add_argument("--expected-count", type=int)

    build_parser = subparsers.add_parser(
        "build", help="build binary glyph metrics from YAML"
    )
    build_parser.add_argument("source", type=Path)
    build_parser.add_argument("output", type=Path)
    build_parser.add_argument("--expected-count", type=int)

    validate_parser = subparsers.add_parser(
        "validate", help="validate glyph-metrics YAML files"
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
            metrics = GlyphMetrics.parse_binary(
                args.source.read_bytes(), args.expected_count
            )
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(
                dump_metrics_yaml(metrics), encoding="ascii", newline="\n"
            )
            print(f"Wrote {args.output}")
        elif args.command == "build":
            metrics = load_metrics_yaml(args.source, args.expected_count)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(metrics.to_bytes())
            print(f"Wrote {args.output}")
        elif args.command == "validate":
            for path in args.paths:
                metrics = load_metrics_yaml(path)
                print(f"{_summary(path, metrics)} [valid]")
        elif args.command == "roundtrip":
            metrics = validate_roundtrip(
                args.source, args.binary, args.expected_count
            )
            print(f"{_summary(args.source, metrics)} [round-trip OK]")
    except (OSError, GlyphMetricsError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
