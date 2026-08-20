#!/usr/bin/env python3
"""Unit tests for compact UV rectangle tables."""

import unittest

from uv_rect_table import UvRectTable, UvRectTableError


class UvRectTableTests(unittest.TestCase):
    def test_binary_yaml_binary_roundtrip(self):
        source = bytes((0, 17, 4, 6, 4, 6))
        table = UvRectTable.parse_binary(source, unit_pixels=8, expected_count=1)

        rebuilt = UvRectTable.parse_document(table.document(), expected_count=1)

        self.assertEqual(rebuilt.to_bytes(), source)
        self.assertEqual(rebuilt.entries[0].v, 136)
        self.assertEqual(rebuilt.entries[0].width, 32)

    def test_rejects_wrong_record_count(self):
        with self.assertRaisesRegex(UvRectTableError, "expected 2"):
            UvRectTable.parse_binary(bytes(6), unit_pixels=8, expected_count=2)

    def test_rejects_nonmultiple_pixel_value(self):
        table = UvRectTable.parse_binary(bytes(6), unit_pixels=8)
        document = table.document()
        document["entries"][0]["width"] = 7
        with self.assertRaisesRegex(UvRectTableError, "divisible"):
            UvRectTable.parse_document(document)


if __name__ == "__main__":
    unittest.main()
