#!/usr/bin/env python3
"""Unit tests for the PSX sprite layout YAML asset format."""

import tempfile
import unittest
from pathlib import Path

import yaml

from sprite_layout import (
    FORMAT_NAME,
    FORMAT_VERSION,
    SpriteLayout,
    SpriteLayoutError,
    dump_layout_yaml,
    parse_layout_binary,
)


SOURCE = (
    b"\x0D\x00\x00\x00\x60\x00\x48\x00"
    b"\x02\x00\x00\x00\xF8\xFF\x10\x00"
)


class SpriteLayoutTests(unittest.TestCase):
    def test_binary_yaml_binary_roundtrip(self):
        layout = parse_layout_binary(SOURCE, expected_count=2)
        document = yaml.safe_load(dump_layout_yaml(layout))
        rebuilt = SpriteLayout.parse_document(document, expected_count=2)

        self.assertEqual(layout.sprites[0].glyph_id, 13)
        self.assertEqual(layout.sprites[1].x, -8)
        self.assertEqual(rebuilt.to_bytes(), SOURCE)

    def test_rejects_partial_record(self):
        with self.assertRaisesRegex(SpriteLayoutError, "not divisible"):
            parse_layout_binary(SOURCE + b"\x00")

    def test_rejects_wrong_record_count(self):
        with self.assertRaisesRegex(SpriteLayoutError, "expected 3"):
            parse_layout_binary(SOURCE, expected_count=3)

    def test_rejects_unknown_yaml_field(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "record_count": 1,
            "sprites": [{"glyph_id": 1, "x": 2, "y": 3, "unknown": 4}],
        }

        with self.assertRaisesRegex(SpriteLayoutError, "exactly glyph_id"):
            SpriteLayout.parse_document(document)

    def test_rejects_coordinate_overflow(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "record_count": 1,
            "sprites": [{"glyph_id": 1, "x": 0x8000, "y": 0}],
        }

        with self.assertRaisesRegex(SpriteLayoutError, "outside"):
            SpriteLayout.parse_document(document)

    def test_dump_is_ascii_and_loadable(self):
        layout = parse_layout_binary(SOURCE)
        text = dump_layout_yaml(layout)

        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "layout.yaml"
            path.write_text(text, encoding="ascii")
            document = yaml.safe_load(path.read_text(encoding="ascii"))

        self.assertEqual(document["format"], FORMAT_NAME)
        self.assertEqual(document["record_count"], 2)
        self.assertEqual(len(document["sprites"]), 2)

    def test_rejects_declared_record_count_mismatch(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "record_count": 2,
            "sprites": [{"glyph_id": 1, "x": 2, "y": 3}],
        }

        with self.assertRaisesRegex(SpriteLayoutError, "record_count"):
            SpriteLayout.parse_document(document)


if __name__ == "__main__":
    unittest.main()
