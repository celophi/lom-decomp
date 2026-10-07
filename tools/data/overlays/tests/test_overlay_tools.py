"""Tests for the overlay resource readers, using synthetic data only."""

import struct
import tempfile
import unittest
import zlib
from pathlib import Path

from tools.data.overlays import icon_set, png, symbols, text_table

US = text_table.TWO_BYTE_CODES["us"]
JP = text_table.TWO_BYTE_CODES["jp"]


def make_table(strings: list[bytes]) -> bytes:
    offsets = []
    body = b""
    base = len(strings) * 2
    for string in strings:
        offsets.append(base + len(body))
        body += string + b"\x00"
    return b"".join(struct.pack("<H", offset) for offset in offsets) + body


class TextTableTest(unittest.TestCase):
    def test_reads_every_entry(self):
        table = text_table.parse(make_table([b"Load?", b"Save"]), 0, US)
        self.assertEqual([entry.text for entry in table.entries], ["Load?", "Save"])
        self.assertEqual(table.size, 4 + 6 + 5)

    def test_zero_after_a_two_byte_code_does_not_end_the_string(self):
        table = text_table.parse(make_table([b"been\x1f\x00matted."]), 0, US)
        self.assertEqual(table.entries[0].text, "been{1F 00}matted.")

    def test_japanese_rule_covers_every_lead_byte(self):
        data = make_table([b"\x1a\x00ab"])
        self.assertEqual(text_table.parse(data, 0, JP).entries[0].text, "{1A 00}ab")
        self.assertEqual(text_table.parse(data, 0, US).entries[0].text, "{1A}")

    def test_decode_escapes_codes_and_braces(self):
        self.assertEqual(text_table.decode(b"a\x16{}\xe5", US), "a{16}{7B}{7D}{E5}")

    def test_rejects_an_odd_first_offset(self):
        with self.assertRaises(ValueError):
            text_table.parse(b"\x03\x00abc\x00", 0, US)


class IconSetTest(unittest.TestCase):
    def make_set(self) -> bytes:
        palette = [0x0000, 0x001F, 0x03E0, 0x7C00] + [0x7FFF] * 12
        pixels = bytes([0x21, 0x03]) + bytes(icon_set.PIXEL_BYTES - 2)
        image = b"".join(struct.pack("<H", color) for color in palette) + pixels
        return struct.pack("<II", 1, 8) + image

    def test_reads_palette_and_pixels(self):
        icons = icon_set.parse(self.make_set(), 0)
        self.assertEqual(len(icons.icons), 1)
        self.assertEqual(icons.size, 8 + icon_set.IMAGE_BYTES)
        self.assertEqual(icons.icons[0].palette[1], 0x001F)

    def test_low_nibble_is_the_left_pixel_and_zero_is_transparent(self):
        rgba = icon_set.parse(self.make_set(), 0).icons[0].rgba()
        self.assertEqual(rgba[0], (255, 0, 0, 255))
        self.assertEqual(rgba[1], (0, 255, 0, 255))
        self.assertEqual(rgba[2], (0, 0, 255, 255))
        self.assertEqual(rgba[3][3], 0)

    def test_rejects_an_offset_inside_the_offset_table(self):
        with self.assertRaises(ValueError):
            icon_set.parse(struct.pack("<II", 1, 4) + bytes(icon_set.IMAGE_BYTES), 0)


class PngTest(unittest.TestCase):
    def test_encodes_size_and_pixels(self):
        data = png.encode_rgba(2, 1, [(1, 2, 3, 4), (5, 6, 7, 8)])
        self.assertEqual(data[:8], b"\x89PNG\r\n\x1a\n")
        width, height = struct.unpack(">II", data[16:24])
        self.assertEqual((width, height), (2, 1))
        start = data.index(b"IDAT") + 4
        length = struct.unpack(">I", data[start - 8 : start - 4])[0]
        pixels = zlib.decompress(data[start : start + length])
        self.assertEqual(pixels, b"\x00\x01\x02\x03\x04\x05\x06\x07\x08")

    def test_rejects_the_wrong_pixel_count(self):
        with self.assertRaises(ValueError):
            png.encode_rgba(2, 2, [(0, 0, 0, 0)])


class SymbolsTest(unittest.TestCase):
    def test_skips_commented_lines(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "symbols.txt"
            path.write_text("a = 0x80140000;\n// b = 0x80140004;\nc   = 0x8014000C; // note\n")
            self.assertEqual(symbols.load(path), {"a": 0x80140000, "c": 0x8014000C})


if __name__ == "__main__":
    unittest.main()
