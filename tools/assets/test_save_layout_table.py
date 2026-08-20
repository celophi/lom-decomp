#!/usr/bin/env python3
"""Unit tests for TITLE save-layout primitive descriptor tables."""

import struct
import unittest

from save_layout_table import SaveLayoutTable, SaveLayoutTableError


class SaveLayoutTableTests(unittest.TestCase):
    def test_binary_yaml_binary_roundtrip(self):
        source = struct.pack(
            "<BBBBhhhhHHHHI",
            7,
            1,
            1,
            0,
            -124,
            64,
            -124,
            64,
            0,
            32,
            96,
            112,
            0,
        )
        textures = ("save_layout_tim_00", "save_layout_tim_01")
        table = SaveLayoutTable.parse_binary(source, textures, 1)
        rebuilt = SaveLayoutTable.parse_document(table.document(), 1, textures)

        self.assertEqual(rebuilt.to_bytes(), source)
        self.assertEqual(rebuilt.entries[0].primitive, "sprite_strip")
        self.assertEqual(rebuilt.entries[0].texture, "save_layout_tim_01")
        self.assertTrue(rebuilt.entries[0].semi_transparent)

    def test_rejects_nonzero_storage_padding(self):
        source = struct.pack(
            "<BBBBhhhhHHHHI", 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0
        )
        with self.assertRaisesRegex(SaveLayoutTableError, "padding byte"):
            SaveLayoutTable.parse_binary(source, ("texture",), 1)

    def test_rejects_unknown_texture(self):
        source = struct.pack(
            "<BBBBhhhhHHHHI", 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0
        )
        with self.assertRaisesRegex(SaveLayoutTableError, "out of range"):
            SaveLayoutTable.parse_binary(source, ("texture",), 1)


if __name__ == "__main__":
    unittest.main()
