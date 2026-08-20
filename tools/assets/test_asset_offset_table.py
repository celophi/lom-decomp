#!/usr/bin/env python3
"""Unit tests for counted self-relative asset-offset tables."""

import struct
import unittest

from asset_offset_table import AssetOffsetTable, AssetOffsetTableError


class AssetOffsetTableTests(unittest.TestCase):
    def test_binary_yaml_binary_roundtrip(self):
        source = struct.pack("<III", 2, 0xC, 0x822C)
        table = AssetOffsetTable.parse_binary(
            source, ("title_menu_tim_0", "title_menu_tim_1")
        )

        rebuilt = AssetOffsetTable.parse_document(table.document())

        self.assertEqual(rebuilt.to_bytes(), source)
        self.assertEqual(rebuilt.entries[1].target, "title_menu_tim_1")

    def test_rejects_incorrect_header_count(self):
        with self.assertRaisesRegex(AssetOffsetTableError, "header declares"):
            AssetOffsetTable.parse_binary(
                struct.pack("<III", 1, 0xC, 0x822C), ("first", "second")
            )

    def test_rejects_offset_inside_table(self):
        with self.assertRaisesRegex(AssetOffsetTableError, "inside the table"):
            AssetOffsetTable.parse_binary(struct.pack("<II", 1, 4), ("asset",))


if __name__ == "__main__":
    unittest.main()
