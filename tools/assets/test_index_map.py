#!/usr/bin/env python3
"""Unit tests for the little-endian u32 index-map format."""

import unittest

import yaml

from index_map import (
    FORMAT_NAME,
    FORMAT_VERSION,
    IndexMap,
    IndexMapError,
    dump_index_map_yaml,
)


SOURCE = b"\x02\x00\x00\x00\xFF\x00\x00\x00\x05\x00\x00\x00"


class IndexMapTests(unittest.TestCase):
    def test_binary_yaml_binary_roundtrip(self):
        index_map = IndexMap.parse_binary(SOURCE, 0xFF, expected_count=3)
        document = yaml.safe_load(dump_index_map_yaml(index_map))
        rebuilt = IndexMap.parse_document(document, expected_count=3)

        self.assertEqual(index_map.entries, (2, None, 5))
        self.assertEqual(rebuilt.to_bytes(), SOURCE)

    def test_rejects_partial_record(self):
        with self.assertRaisesRegex(IndexMapError, "not divisible"):
            IndexMap.parse_binary(SOURCE + b"\x00", 0xFF)

    def test_rejects_wrong_entry_count(self):
        with self.assertRaisesRegex(IndexMapError, "expected 4"):
            IndexMap.parse_binary(SOURCE, 0xFF, expected_count=4)

    def test_rejects_declared_count_mismatch(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "entry_count": 2,
            "unmapped_value": 0xFF,
            "entries": [None],
        }

        with self.assertRaisesRegex(IndexMapError, "entry_count"):
            IndexMap.parse_document(document)

    def test_rejects_literal_sentinel(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "entry_count": 1,
            "unmapped_value": 0xFF,
            "entries": [0xFF],
        }

        with self.assertRaisesRegex(IndexMapError, "must use null"):
            IndexMap.parse_document(document)

    def test_rejects_u32_overflow(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "entry_count": 1,
            "unmapped_value": 0xFF,
            "entries": [0x100000000],
        }

        with self.assertRaisesRegex(IndexMapError, "outside"):
            IndexMap.parse_document(document)

    def test_rejects_unknown_yaml_field(self):
        index_map = IndexMap.parse_binary(SOURCE, 0xFF)
        document = index_map.document()
        document["unknown"] = 1

        with self.assertRaisesRegex(IndexMapError, "unknown keys"):
            IndexMap.parse_document(document)


if __name__ == "__main__":
    unittest.main()
