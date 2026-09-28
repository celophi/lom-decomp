"""Write RGBA PNG images with the standard library only."""

from __future__ import annotations

import struct
import zlib
from pathlib import Path


def _chunk(kind: bytes, payload: bytes) -> bytes:
    return struct.pack(">I", len(payload)) + kind + payload + struct.pack(">I", zlib.crc32(kind + payload) & 0xFFFFFFFF)


def encode_rgba(width: int, height: int, pixels: list[tuple[int, int, int, int]]) -> bytes:
    """Encode row-major RGBA pixels as a PNG file."""
    if len(pixels) != width * height:
        raise ValueError(f"expected {width * height} pixels, got {len(pixels)}")
    rows = bytearray()
    for y in range(height):
        rows.append(0)  # filter type: none
        for red, green, blue, alpha in pixels[y * width : (y + 1) * width]:
            rows += bytes((red, green, blue, alpha))
    header = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    return b"\x89PNG\r\n\x1a\n" + _chunk(b"IHDR", header) + _chunk(b"IDAT", zlib.compress(bytes(rows), 9)) + _chunk(b"IEND", b"")


def write_rgba(path: Path, width: int, height: int, pixels: list[tuple[int, int, int, int]]) -> None:
    path.write_bytes(encode_rgba(width, height, pixels))
