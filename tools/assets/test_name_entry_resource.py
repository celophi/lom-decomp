#!/usr/bin/env python3
"""Unit tests for the GNAME offset-table record archive."""

import struct
import unittest

import yaml

from name_entry_resource import (
    FORMAT_NAME,
    FORMAT_VERSION,
    NameEntryResource,
    NameEntryResourceError,
    dump_resource_yaml,
)


def _table(*records: bytes) -> bytes:
    offset = len(records) * 2
    offsets = []
    for record in records:
        offsets.append(offset)
        offset += len(record)
    return b"".join(struct.pack("<H", value) for value in offsets) + b"".join(records)


TABLES = (
    _table(b"A", b"BC"),
    _table(b""),
    _table(b"\x00"),
    _table(b"\xFF\x01"),
)
OFFSETS = (0x14, 0x1B, 0x1D, 0x20)
SOURCE = struct.pack("<IIIII", 4, *OFFSETS) + b"".join(TABLES)


class NameEntryResourceTests(unittest.TestCase):
    def test_binary_yaml_binary_roundtrip(self):
        resource = NameEntryResource.parse_binary(SOURCE)
        document = yaml.safe_load(dump_resource_yaml(resource))
        rebuilt = NameEntryResource.parse_document(document)

        self.assertEqual(resource.unknown_0x00, 4)
        self.assertEqual(len(resource.tables[0].records), 2)
        self.assertEqual(resource.tables[0].records[1], b"BC")
        self.assertEqual(rebuilt.to_bytes(), SOURCE)

    def test_rejects_wrong_first_table_offset(self):
        source = bytearray(SOURCE)
        struct.pack_into("<I", source, 4, 0x18)
        with self.assertRaisesRegex(NameEntryResourceError, "first table offset"):
            NameEntryResource.parse_binary(bytes(source))

    def test_rejects_decreasing_header_offsets(self):
        source = bytearray(SOURCE)
        struct.pack_into("<I", source, 8, 0x13)
        with self.assertRaisesRegex(NameEntryResourceError, "predecessor"):
            NameEntryResource.parse_binary(bytes(source))

    def test_rejects_decreasing_record_offsets(self):
        bad_table = b"\x06\x00\x08\x00\x07\x00AA"
        tables = (bad_table, TABLES[1], TABLES[2], TABLES[3])
        offsets = [0x14]
        for table in tables[:-1]:
            offsets.append(offsets[-1] + len(table))
        source = struct.pack("<IIIII", 4, *offsets) + b"".join(tables)

        with self.assertRaisesRegex(NameEntryResourceError, "less than"):
            NameEntryResource.parse_binary(source)

    def test_rejects_record_count_mismatch(self):
        resource = NameEntryResource.parse_binary(SOURCE)
        document = resource.document()
        document["tables"]["panel_records"]["record_count"] = 3

        with self.assertRaisesRegex(NameEntryResourceError, "record_count"):
            NameEntryResource.parse_document(document)

    def test_rejects_invalid_hex_record(self):
        resource = NameEntryResource.parse_binary(SOURCE)
        document = resource.document()
        document["tables"]["panel_records"]["records"][0] = {"hex": "GG"}

        with self.assertRaisesRegex(NameEntryResourceError, "hexadecimal"):
            NameEntryResource.parse_document(document)

    def test_extracts_plain_ascii_records_as_text(self):
        tables = (
            _table(b"Deletes one letter.\x00"),
            _table(b"\x1D\xD4\x00"),
            _table(b"Rabite\x00"),
            _table(b"Maya\x00\x00"),
        )
        offsets = [0x14]
        for table in tables[:-1]:
            offsets.append(offsets[-1] + len(table))
        source = struct.pack("<IIIII", 4, *offsets) + b"".join(tables)

        resource = NameEntryResource.parse_binary(source)
        document = resource.document()

        self.assertEqual(
            document["tables"]["panel_records"]["records"][0],
            {"text": "Deletes one letter."},
        )
        self.assertEqual(
            document["tables"]["history_names"]["records"][0],
            {"text": "Rabite"},
        )
        self.assertEqual(
            document["tables"]["random_names"]["records"][0],
            {"text": "Maya", "trailing_zero_bytes": 2},
        )
        self.assertEqual(NameEntryResource.parse_document(document).to_bytes(), source)

    def test_extracts_dictionary_tokens_as_plain_text(self):
        tables = (
            _table(b"F\x16alize\x15r name.\x00"),
            _table(b"\x19\x00\x00"),
            _table(b"\x00"),
            _table(b"\x00"),
        )
        offsets = [0x14]
        for table in tables[:-1]:
            offsets.append(offsets[-1] + len(table))
        source = struct.pack("<IIIII", 4, *offsets) + b"".join(tables)

        document = NameEntryResource.parse_binary(source).document()

        self.assertEqual(
            document["tables"]["panel_records"]["records"][0],
            {"text": "Finalize your name.", "encoding": "dictionary"},
        )
        self.assertEqual(
            document["tables"]["kanji_records"]["records"][0],
            {"glyph_codes": ["1900"]},
        )
        self.assertEqual(NameEntryResource.parse_document(document).to_bytes(), source)

    def test_rejects_unmapped_unicode_text(self):
        resource = NameEntryResource.parse_binary(SOURCE)
        document = resource.document()
        document["tables"]["history_names"]["records"][0] = {"text": "\u30E9\u30D3"}

        with self.assertRaisesRegex(NameEntryResourceError, "verified GNAME encoding"):
            NameEntryResource.parse_document(document)

    def test_rejects_dictionary_encoding_outside_panel_table(self):
        resource = NameEntryResource.parse_binary(SOURCE)
        document = resource.document()
        document["tables"]["history_names"]["records"][0] = {
            "text": "Finalize your name.",
            "encoding": "dictionary",
        }

        with self.assertRaisesRegex(NameEntryResourceError, "only valid"):
            NameEntryResource.parse_document(document)

    def test_rejects_unknown_yaml_field(self):
        resource = NameEntryResource.parse_binary(SOURCE)
        document = resource.document()
        document["unknown"] = 1

        with self.assertRaisesRegex(NameEntryResourceError, "unknown keys"):
            NameEntryResource.parse_document(document)

    def test_schema_identifiers(self):
        resource = NameEntryResource.parse_binary(SOURCE)
        document = resource.document()

        self.assertEqual(document["format"], FORMAT_NAME)
        self.assertEqual(document["version"], FORMAT_VERSION)


if __name__ == "__main__":
    unittest.main()
