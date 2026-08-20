#!/usr/bin/env python3
"""Unit tests for editable u8 sequences."""

import unittest

from u8_sequence import U8Sequence, U8SequenceError


class U8SequenceTests(unittest.TestCase):
    def test_binary_yaml_binary_roundtrip(self):
        source = bytes((0, 16, 32, 16))
        sequence = U8Sequence.parse_binary(source, expected_count=4)

        rebuilt = U8Sequence.parse_document(sequence.document(), expected_count=4)

        self.assertEqual(rebuilt.to_bytes(), source)

    def test_rejects_wrong_expected_count(self):
        with self.assertRaisesRegex(U8SequenceError, "expected 4"):
            U8Sequence.parse_binary(b"\x00\x10", expected_count=4)

    def test_rejects_value_outside_u8(self):
        document = U8Sequence((0,)).document()
        document["values"] = [256]
        with self.assertRaisesRegex(U8SequenceError, "values\[0\]"):
            U8Sequence.parse_document(document)


if __name__ == "__main__":
    unittest.main()
