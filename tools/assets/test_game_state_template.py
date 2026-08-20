#!/usr/bin/env python3
"""Unit tests for partially mapped game-state templates."""

import struct
import unittest

from game_state_template import GameStateTemplate, GameStateTemplateError


class GameStateTemplateTests(unittest.TestCase):
    def make_source(self):
        source = bytearray(0x620)
        struct.pack_into("<Ihb", source, 0x18, 0x12345678, -2, -1)
        struct.pack_into("<I", source, 0x20, 3)
        struct.pack_into("<HBBI", source, 0x24, 13, 2, 1, 0xC)
        struct.pack_into("<11I", source, 0x34, *range(11))
        struct.pack_into("<h", source, 0xD4, 42)
        struct.pack_into("<I", source, 0x2E0, 1)
        struct.pack_into("<I", source, 0x608, 0x80)
        return bytes(source)

    def test_binary_yaml_binary_roundtrip(self):
        source = self.make_source()
        template = GameStateTemplate.parse_binary(
            source, "template.payload.bin", len(source), 0x610
        )
        rebuilt = GameStateTemplate.parse_document(
            template.document(), source, len(source), "template.payload.bin", 0x610
        )

        self.assertEqual(rebuilt.to_bytes(), source)
        self.assertEqual(rebuilt.copied_size, 0x610)
        self.assertEqual(rebuilt.option_id, -2)
        self.assertEqual(rebuilt.weapon_category_masks[10], 10)

    def test_yaml_field_change_patches_payload(self):
        source = self.make_source()
        template = GameStateTemplate.parse_binary(
            source, "template.payload.bin", len(source), 0x610
        )
        document = template.document()
        document["known_fields"]["scene_mode"] = 7
        rebuilt = GameStateTemplate.parse_document(document, source)

        self.assertEqual(struct.unpack_from("<H", rebuilt.to_bytes(), 0x24)[0], 7)

    def test_rejects_wrong_payload_size(self):
        with self.assertRaisesRegex(GameStateTemplateError, "payload size"):
            GameStateTemplate.parse_binary(
                bytes(0x620), "template.payload.bin", 0x624, 0x610
            )

    def test_rejects_copied_size_beyond_template(self):
        with self.assertRaisesRegex(GameStateTemplateError, "cannot exceed"):
            GameStateTemplate.parse_binary(
                bytes(0x620), "template.payload.bin", 0x620, 0x624
            )


if __name__ == "__main__":
    unittest.main()
