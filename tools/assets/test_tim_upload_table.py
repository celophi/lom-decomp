#!/usr/bin/env python3
"""Unit tests for PSX TIM upload descriptor tables."""

import struct
import unittest

from tim_upload_table import TimUploadTable, TimUploadTableError


class TimUploadTableTests(unittest.TestCase):
    def test_binary_yaml_binary_roundtrip(self):
        source = struct.pack("<hhhhII", 320, 0, 0, 480, 0x8007FD30, 0)
        table = TimUploadTable.parse_binary(source, ("save_layout_tim_00",))

        rebuilt = TimUploadTable.parse_document(table.document())

        self.assertEqual(rebuilt.to_bytes(), source)
        self.assertEqual(rebuilt.entries[0].clut_y, 480)

    def test_rejects_target_count_mismatch(self):
        source = struct.pack("<hhhhII", 320, 0, 0, 480, 0x8007FD30, 0)
        with self.assertRaisesRegex(TimUploadTableError, "target names"):
            TimUploadTable.parse_binary(source, ())

    def test_rejects_unaligned_source_address(self):
        source = struct.pack("<hhhhII", 320, 0, 0, 480, 0x8007FD31, 0)
        with self.assertRaisesRegex(TimUploadTableError, "4-byte aligned"):
            TimUploadTable.parse_binary(source, ("save_layout_tim_00",))


if __name__ == "__main__":
    unittest.main()
