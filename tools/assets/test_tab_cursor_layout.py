#!/usr/bin/env python3
"""Unit tests for the packed PSX tab-cursor layout format."""

import unittest

import yaml

from tab_cursor_layout import (
    FORMAT_NAME,
    FORMAT_VERSION,
    TabCursorLayout,
    TabCursorLayoutError,
    dump_layout_yaml,
)


SOURCE = b"\xFF\xFF\xC8\x05\x20\x07\x08\x18"


class TabCursorLayoutTests(unittest.TestCase):
    def test_binary_yaml_binary_roundtrip(self):
        layout = TabCursorLayout.parse_binary(SOURCE, expected_count=2)
        document = yaml.safe_load(dump_layout_yaml(layout))
        rebuilt = TabCursorLayout.parse_document(document, expected_count=2)

        self.assertEqual(layout.entries[0].x, 0x1FF)
        self.assertEqual(layout.entries[0].sprite_index, 0x7F)
        self.assertEqual(layout.entries[1].x, 0x120)
        self.assertEqual(layout.entries[1].sprite_index, 3)
        self.assertEqual(rebuilt.to_bytes(), SOURCE)

    def test_rejects_partial_record(self):
        with self.assertRaisesRegex(TabCursorLayoutError, "not divisible"):
            TabCursorLayout.parse_binary(SOURCE + b"\x00")

    def test_rejects_wrong_record_count(self):
        with self.assertRaisesRegex(TabCursorLayoutError, "expected 3"):
            TabCursorLayout.parse_binary(SOURCE, expected_count=3)

    def test_rejects_declared_count_mismatch(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "record_count": 2,
            "entries": [
                {"x": 1, "sprite_index": 2, "y": 3, "glyph_id": 4}
            ],
        }

        with self.assertRaisesRegex(TabCursorLayoutError, "record_count"):
            TabCursorLayout.parse_document(document)

    def test_rejects_x_bitfield_overflow(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "record_count": 1,
            "entries": [
                {"x": 0x200, "sprite_index": 2, "y": 3, "glyph_id": 4}
            ],
        }

        with self.assertRaisesRegex(TabCursorLayoutError, "outside"):
            TabCursorLayout.parse_document(document)

    def test_rejects_sprite_index_overflow(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "record_count": 1,
            "entries": [
                {"x": 1, "sprite_index": 0x80, "y": 3, "glyph_id": 4}
            ],
        }

        with self.assertRaisesRegex(TabCursorLayoutError, "outside"):
            TabCursorLayout.parse_document(document)

    def test_rejects_unknown_yaml_field(self):
        layout = TabCursorLayout.parse_binary(SOURCE)
        document = layout.document()
        document["entries"][0]["unknown"] = 1

        with self.assertRaisesRegex(TabCursorLayoutError, "exactly x"):
            TabCursorLayout.parse_document(document)


if __name__ == "__main__":
    unittest.main()
