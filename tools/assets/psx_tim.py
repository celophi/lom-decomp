#!/usr/bin/env python3
"""Validate and rebuild standard PlayStation TIM image files."""

from __future__ import annotations

import argparse
import json
import struct
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


TIM_MAGIC = 0x10
TIM_HEADER_SIZE = 8
TIM_BLOCK_HEADER_SIZE = 12
TIM_FLAG_HAS_CLUT = 0x08
TIM_PIXEL_MODE_MASK = 0x07

PIXEL_MODE_NAMES = {
    0: "4bpp",
    1: "8bpp",
    2: "16bpp",
    3: "24bpp",
}


class TimFormatError(ValueError):
    """Raised when a byte stream is not a structurally valid TIM file."""


@dataclass(frozen=True)
class TimBlock:
    """One TIM CLUT or pixel block, including its VRAM placement metadata."""

    x: int
    y: int
    width_words: int
    height: int
    payload: bytes

    @classmethod
    def parse(cls, data: bytes, offset: int, label: str) -> tuple["TimBlock", int]:
        if len(data) - offset < TIM_BLOCK_HEADER_SIZE:
            raise TimFormatError(
                f"{label} block header at 0x{offset:X} is truncated"
            )

        size, x, y, width_words, height = struct.unpack_from("<IHHHH", data, offset)
        if size < TIM_BLOCK_HEADER_SIZE:
            raise TimFormatError(
                f"{label} block at 0x{offset:X} has invalid size 0x{size:X}"
            )

        end = offset + size
        if end > len(data):
            raise TimFormatError(
                f"{label} block at 0x{offset:X} ends at 0x{end:X}, "
                f"past file size 0x{len(data):X}"
            )

        expected_payload_size = width_words * height * 2
        actual_payload_size = size - TIM_BLOCK_HEADER_SIZE
        if actual_payload_size != expected_payload_size:
            raise TimFormatError(
                f"{label} block at 0x{offset:X} declares 0x{actual_payload_size:X} "
                f"payload bytes, but {width_words}x{height} VRAM words require "
                f"0x{expected_payload_size:X}"
            )

        payload = data[offset + TIM_BLOCK_HEADER_SIZE : end]
        return cls(x, y, width_words, height, payload), end

    def to_bytes(self) -> bytes:
        expected_payload_size = self.width_words * self.height * 2
        if len(self.payload) != expected_payload_size:
            raise TimFormatError(
                f"block payload is 0x{len(self.payload):X} bytes, expected "
                f"0x{expected_payload_size:X}"
            )

        size = TIM_BLOCK_HEADER_SIZE + len(self.payload)
        return struct.pack(
            "<IHHHH", size, self.x, self.y, self.width_words, self.height
        ) + self.payload

    def metadata(self) -> dict[str, int]:
        return {
            "size": TIM_BLOCK_HEADER_SIZE + len(self.payload),
            "x": self.x,
            "y": self.y,
            "width_words": self.width_words,
            "height": self.height,
            "payload_size": len(self.payload),
        }


@dataclass(frozen=True)
class TimImage:
    """Parsed TIM file that can serialize itself without losing metadata."""

    flags: int
    clut: TimBlock | None
    pixels: TimBlock

    @classmethod
    def parse(cls, data: bytes) -> "TimImage":
        if len(data) < TIM_HEADER_SIZE:
            raise TimFormatError("TIM header is truncated")

        magic, flags = struct.unpack_from("<II", data, 0)
        if magic != TIM_MAGIC:
            raise TimFormatError(
                f"invalid TIM magic 0x{magic:08X}; expected 0x{TIM_MAGIC:08X}"
            )

        pixel_mode = flags & TIM_PIXEL_MODE_MASK
        if pixel_mode not in PIXEL_MODE_NAMES:
            raise TimFormatError(f"unsupported TIM pixel mode {pixel_mode}")

        offset = TIM_HEADER_SIZE
        clut = None
        if flags & TIM_FLAG_HAS_CLUT:
            clut, offset = TimBlock.parse(data, offset, "CLUT")

        pixels, offset = TimBlock.parse(data, offset, "pixel")
        if offset != len(data):
            raise TimFormatError(
                f"TIM has 0x{len(data) - offset:X} trailing bytes at 0x{offset:X}"
            )

        return cls(flags, clut, pixels)

    @property
    def pixel_mode(self) -> int:
        return self.flags & TIM_PIXEL_MODE_MASK

    def to_bytes(self) -> bytes:
        data = bytearray(struct.pack("<II", TIM_MAGIC, self.flags))
        if self.clut is not None:
            data.extend(self.clut.to_bytes())
        data.extend(self.pixels.to_bytes())
        return bytes(data)

    def metadata(self) -> dict[str, object]:
        return {
            "magic": TIM_MAGIC,
            "flags": self.flags,
            "pixel_mode": self.pixel_mode,
            "pixel_mode_name": PIXEL_MODE_NAMES[self.pixel_mode],
            "has_clut": self.clut is not None,
            "clut": self.clut.metadata() if self.clut is not None else None,
            "pixels": self.pixels.metadata(),
            "file_size": len(self.to_bytes()),
        }


def parse_tim(data: bytes) -> TimImage:
    """Parse and strictly validate one complete TIM byte stream."""
    return TimImage.parse(data)


def load_tim(path: Path) -> TimImage:
    """Read and parse a TIM file from disk."""
    return parse_tim(path.read_bytes())


def validate_roundtrip(path: Path) -> TimImage:
    """Confirm that parsing and rebuilding a TIM reproduces every source byte."""
    source = path.read_bytes()
    image = parse_tim(source)
    rebuilt = image.to_bytes()
    if rebuilt != source:
        raise TimFormatError(f"{path}: rebuilt TIM differs from source bytes")
    return image


def _summary(path: Path, image: TimImage) -> str:
    clut = "with CLUT" if image.clut is not None else "without CLUT"
    pixels = image.pixels
    return (
        f"{path}: {PIXEL_MODE_NAMES[image.pixel_mode]}, {clut}, "
        f"image {pixels.width_words}x{pixels.height} VRAM words, "
        f"0x{len(image.to_bytes()):X} bytes"
    )


def _validate_paths(paths: Iterable[Path], roundtrip: bool) -> None:
    for path in paths:
        image = validate_roundtrip(path) if roundtrip else load_tim(path)
        action = "round-trip OK" if roundtrip else "valid"
        print(f"{_summary(path, image)} [{action}]")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    validate_parser = subparsers.add_parser("validate", help="validate TIM files")
    validate_parser.add_argument("paths", nargs="+", type=Path)

    roundtrip_parser = subparsers.add_parser(
        "roundtrip", help="parse, rebuild, and byte-compare TIM files"
    )
    roundtrip_parser.add_argument("paths", nargs="+", type=Path)

    info_parser = subparsers.add_parser("info", help="print TIM metadata")
    info_parser.add_argument("path", type=Path)
    info_parser.add_argument("--json", action="store_true")

    build_parser = subparsers.add_parser(
        "build", help="validate and rebuild a TIM into a new file"
    )
    build_parser.add_argument("source", type=Path)
    build_parser.add_argument("output", type=Path)

    args = parser.parse_args()

    try:
        if args.command == "validate":
            _validate_paths(args.paths, roundtrip=False)
        elif args.command == "roundtrip":
            _validate_paths(args.paths, roundtrip=True)
        elif args.command == "info":
            image = load_tim(args.path)
            if args.json:
                print(json.dumps(image.metadata(), indent=2))
            else:
                print(_summary(args.path, image))
        elif args.command == "build":
            image = load_tim(args.source)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(image.to_bytes())
            print(f"Wrote {args.output}")
    except (OSError, TimFormatError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
