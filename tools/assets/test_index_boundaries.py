#!/usr/bin/env python3
"""Unit tests for the little-endian u32 index-boundary format."""

import unittest

import yaml

from index_boundaries import (
    FORMAT_NAME,
    FORMAT_VERSION,
    IndexBoundaries,
    IndexBoundariesError,
    dump_boundaries_yaml,
)


SOURCE = b"\x00\x00\x00\x00\x52\x00\x00\x00\x9E\x00\x00\x00"


class IndexBoundariesTests(unittest.TestCase):
    def test_binary_yaml_binary_roundtrip(self):
        boundaries = IndexBoundaries.parse_binary(SOURCE, expected_count=3)
        document = yaml.safe_load(dump_boundaries_yaml(boundaries))
        rebuilt = IndexBoundaries.parse_document(document, expected_count=3)

        self.assertEqual(boundaries.boundaries, (0, 82, 158))
        self.assertEqual(rebuilt.to_bytes(), SOURCE)

    def test_rejects_partial_record(self):
        with self.assertRaisesRegex(IndexBoundariesError, "not divisible"):
            IndexBoundaries.parse_binary(SOURCE + b"\x00")

    def test_rejects_wrong_boundary_count(self):
        with self.assertRaisesRegex(IndexBoundariesError, "expected 4"):
            IndexBoundaries.parse_binary(SOURCE, expected_count=4)

    def test_rejects_decreasing_binary_boundaries(self):
        source = b"\x02\x00\x00\x00\x01\x00\x00\x00"
        with self.assertRaisesRegex(IndexBoundariesError, "less than"):
            IndexBoundaries.parse_binary(source)

    def test_allows_duplicate_boundaries_for_empty_ranges(self):
        source = b"\x01\x00\x00\x00\x01\x00\x00\x00"
        boundaries = IndexBoundaries.parse_binary(source)

        self.assertEqual(boundaries.to_bytes(), source)

    def test_rejects_declared_count_mismatch(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "boundary_count": 2,
            "boundaries": [0],
        }

        with self.assertRaisesRegex(IndexBoundariesError, "boundary_count"):
            IndexBoundaries.parse_document(document)

    def test_rejects_u32_overflow(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "boundary_count": 1,
            "boundaries": [0x100000000],
        }

        with self.assertRaisesRegex(IndexBoundariesError, "outside"):
            IndexBoundaries.parse_document(document)

    def test_rejects_unknown_yaml_field(self):
        boundaries = IndexBoundaries.parse_binary(SOURCE)
        document = boundaries.document()
        document["unknown"] = 1

        with self.assertRaisesRegex(IndexBoundariesError, "unknown keys"):
            IndexBoundaries.parse_document(document)


if __name__ == "__main__":
    unittest.main()
