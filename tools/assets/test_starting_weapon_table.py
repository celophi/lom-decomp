#!/usr/bin/env python3
"""Unit tests for TITLE starting-weapon equipment records."""

import struct
import unittest

from starting_weapon_table import StartingWeaponTable, StartingWeaponTableError


class StartingWeaponTableTests(unittest.TestCase):
    def test_binary_yaml_binary_roundtrip(self):
        name = b"MenosKnife".ljust(0x14, b"\0")
        source = name + struct.pack(
            "<IIIIH", 0, 0, 0x44444444, 0xFFFFFFFF, 9
        ) + bytes.fromhex("000c0d0e0fff0000000003030300320000000000000000000000")
        categories = ("knife",)
        table = StartingWeaponTable.parse_binary(source, categories, 1)
        rebuilt = StartingWeaponTable.parse_document(table.document(), 1, categories)

        self.assertEqual(rebuilt.to_bytes(), source)
        self.assertEqual(rebuilt.entries[0].name, "MenosKnife")
        self.assertEqual(rebuilt.entries[0].category, "knife")
        self.assertEqual(rebuilt.entries[0].primary_value, 9)

    def test_rejects_non_weapon_kind(self):
        source = b"Test".ljust(0x14, b"\0") + struct.pack(
            "<IIIIH", 0x100, 0, 0, 0, 1
        ) + bytes(0x1A)
        with self.assertRaisesRegex(StartingWeaponTableError, "weapon kind 0"):
            StartingWeaponTable.parse_binary(source, ("knife",), 1)

    def test_rejects_bad_opaque_tail_length(self):
        source = b"Test".ljust(0x14, b"\0") + struct.pack(
            "<IIIIH", 0, 0, 0, 0, 1
        ) + bytes(0x1A)
        table = StartingWeaponTable.parse_binary(source, ("knife",), 1)
        document = table.document()
        document["records"][0]["opaque_26_3f"] = "00"
        with self.assertRaisesRegex(StartingWeaponTableError, "0x1A bytes"):
            StartingWeaponTable.parse_document(document)


if __name__ == "__main__":
    unittest.main()
