#!/usr/bin/env python3
"""Unit tests for the PSX TIM parser and byte-exact builder."""

import struct
import unittest

from psx_tim import TIM_MAGIC, TimFormatError, parse_tim


def make_block(x: int, y: int, width_words: int, height: int, fill: int) -> bytes:
    payload = bytes([fill]) * (width_words * height * 2)
    return struct.pack(
        "<IHHHH", 12 + len(payload), x, y, width_words, height
    ) + payload


def make_indexed_tim() -> bytes:
    clut = make_block(0, 480, 16, 1, 0x12)
    pixels = make_block(320, 0, 2, 2, 0x34)
    return struct.pack("<II", TIM_MAGIC, 0x08) + clut + pixels


class TimImageTests(unittest.TestCase):
    def test_indexed_tim_roundtrips_exactly(self):
        source = make_indexed_tim()
        image = parse_tim(source)

        self.assertEqual(image.pixel_mode, 0)
        self.assertIsNotNone(image.clut)
        self.assertEqual(image.pixels.width_words, 2)
        self.assertEqual(image.to_bytes(), source)

    def test_direct_color_tim_without_clut_roundtrips(self):
        pixels = make_block(10, 20, 3, 2, 0x56)
        source = struct.pack("<II", TIM_MAGIC, 0x02) + pixels

        image = parse_tim(source)

        self.assertIsNone(image.clut)
        self.assertEqual(image.pixel_mode, 2)
        self.assertEqual(image.to_bytes(), source)

    def test_rejects_trailing_bytes(self):
        with self.assertRaisesRegex(TimFormatError, "trailing bytes"):
            parse_tim(make_indexed_tim() + b"\x00\x01")

    def test_rejects_truncated_block(self):
        with self.assertRaisesRegex(TimFormatError, "past file size"):
            parse_tim(make_indexed_tim()[:-1])

    def test_rejects_inconsistent_block_dimensions(self):
        source = bytearray(make_indexed_tim())
        struct.pack_into("<H", source, 16, 17)

        with self.assertRaisesRegex(TimFormatError, "VRAM words require"):
            parse_tim(bytes(source))

    def test_rejects_invalid_magic(self):
        source = bytearray(make_indexed_tim())
        struct.pack_into("<I", source, 0, 0x11)

        with self.assertRaisesRegex(TimFormatError, "invalid TIM magic"):
            parse_tim(bytes(source))


if __name__ == "__main__":
    unittest.main()
