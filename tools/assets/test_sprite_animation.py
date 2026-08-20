#!/usr/bin/env python3
"""Unit tests for the PSX sprite animation YAML asset format."""

import unittest

import yaml

from sprite_animation import (
    FORMAT_NAME,
    FORMAT_VERSION,
    SpriteAnimation,
    SpriteAnimationError,
    dump_animation_yaml,
)


SOURCE = (
    b"\x10\x00\x0E\x02\x30\x08\x18\x00\x00\x00\x00\x00"
    b"\x22\x10\x17\x03\x10\x00\x0E\x02\x30\x0C\x19\x00"
    b"\x00\x00\x00\x00"
)


class SpriteAnimationTests(unittest.TestCase):
    def test_binary_yaml_binary_roundtrip(self):
        animation = SpriteAnimation.parse_binary(SOURCE, 2, 3, 4)
        document = yaml.safe_load(dump_animation_yaml(animation))
        rebuilt = SpriteAnimation.parse_document(document)

        self.assertEqual(animation.frames[0].sprites[0].control, 2)
        self.assertEqual(animation.frames[1].sprites[0].glyph_id, 0x17)
        self.assertEqual(animation.frames[1].sprites[1].control, 2)
        self.assertEqual(rebuilt.to_bytes(), SOURCE)

    def test_rejects_wrong_binary_size(self):
        with self.assertRaisesRegex(SpriteAnimationError, "expected 0x1C"):
            SpriteAnimation.parse_binary(SOURCE[:-1], 2, 3, 4)

    def test_rejects_nonzero_trailing_padding(self):
        source = SOURCE[:-1] + b"\x01"
        with self.assertRaisesRegex(SpriteAnimationError, "padding"):
            SpriteAnimation.parse_binary(source, 2, 3, 4)

    def test_rejects_wrong_sprite_count(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "frame_count": 1,
            "sprites_per_frame": 3,
            "trailing_padding": 0,
            "frames": [
                {
                    "sprites": [
                        {"x": 1, "y": 2, "glyph_id": 3, "control": 2}
                    ],
                }
            ],
        }

        with self.assertRaisesRegex(SpriteAnimationError, "expected 3"):
            SpriteAnimation.parse_document(document)

    def test_rejects_byte_overflow(self):
        document = {
            "format": FORMAT_NAME,
            "version": FORMAT_VERSION,
            "frame_count": 1,
            "sprites_per_frame": 1,
            "trailing_padding": 0,
            "frames": [
                {
                    "sprites": [
                        {"x": 256, "y": 2, "glyph_id": 3, "control": 2}
                    ],
                }
            ],
        }

        with self.assertRaisesRegex(SpriteAnimationError, "outside"):
            SpriteAnimation.parse_document(document)

    def test_rejects_unknown_yaml_field(self):
        animation = SpriteAnimation.parse_binary(SOURCE, 2, 3, 4)
        document = animation.document()
        document["unknown"] = 1

        with self.assertRaisesRegex(SpriteAnimationError, "unknown keys"):
            SpriteAnimation.parse_document(document)


if __name__ == "__main__":
    unittest.main()
