#!/usr/bin/env python3
"""Unit tests for the PSX glyph-metrics YAML asset format."""

import unittest

import yaml

from glyph_metrics import (
    FORMAT_NAME,
    FORMAT_VERSION,
    GlyphMetrics,
    GlyphMetricsError,
    dump_metrics_yaml,
)


SOURCE = (
    b"\x40\x88\x60\x60\x01\x00\x00\x00"
    b"\x00\x00\x40\xE0\x00\x00\x00\x00"
)


class GlyphMetricsTests(unittest.TestCase):
    def test_binary_yaml_binary_roundtrip(self):
        metrics = GlyphMetrics.parse_binary(SOURCE, expected_count=2)
        document = yaml.safe_load(dump_metrics_yaml(metrics))
        rebuilt = GlyphMetrics.parse_document(document, expected_count=2)

        self.assertEqual(metrics.glyphs[0].u, 0x40)
        self.assertEqual(metrics.glyphs[0].clut, 1)
        self.assertEqual(metrics.glyphs[1].height, 0xE0)
        self.assertEqual(rebuilt.to_bytes(), SOURCE)

    def test_rejects_partial_record(self):
        with self.assertRaisesRegex(GlyphMetricsError, "not divisible"):
            GlyphMetrics.parse_binary(SOURCE + b"\x00")

    def test_rejects_wrong_record_count(self):
        with self.assertRaisesRegex(GlyphMetricsError, "expected 3"):
            GlyphMetrics.parse_binary(SOURCE, expected_count=3)

    def test_rejects_declared_count_mismatch(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "record_count": 2,
            "glyphs": [
                {"u": 1, "v": 2, "width": 3, "height": 4, "clut": 5}
            ],
        }

        with self.assertRaisesRegex(GlyphMetricsError, "record_count"):
            GlyphMetrics.parse_document(document)

    def test_rejects_byte_overflow(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "record_count": 1,
            "glyphs": [
                {"u": 256, "v": 2, "width": 3, "height": 4, "clut": 5}
            ],
        }

        with self.assertRaisesRegex(GlyphMetricsError, "outside"):
            GlyphMetrics.parse_document(document)

    def test_rejects_unknown_yaml_field(self):
        metrics = GlyphMetrics.parse_binary(SOURCE)
        document = metrics.document()
        document["glyphs"][0]["unknown"] = 1

        with self.assertRaisesRegex(GlyphMetricsError, "exactly u"):
            GlyphMetrics.parse_document(document)


if __name__ == "__main__":
    unittest.main()
