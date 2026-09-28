#!/usr/bin/env python3
"""Tests for FIELD scene extraction using synthetic data, not game assets."""

import json
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from tools.scenes.field_scene import FieldScene, SceneFormatError


ROOT = Path(__file__).resolve().parents[3]


def make_scene(chest: bool = True) -> bytes:
    """@brief Assemble a synthetic scene with an indexed TIM and one entry.
    @param chest Whether to include the recognized initializer instructions.
    @return Scene bytes with valid section boundaries and counts.
    """
    initializer = bytes([0x40, 0x84, 0x20, 0xC0])
    for field_index, destination in ((8, 0xE040), (9, 0xE050)):
        initializer += bytes([0x0C, 1, 2, 0xFF, 0x10, 0, field_index, 0x40])
        initializer += struct.pack("<H", destination)
    if not chest:
        initializer = bytes(len(initializer))
    events = struct.pack("<2H", 4, 4) + initializer
    scripts = [0xFFFF] * 16
    scripts[4], scripts[5], scripts[15] = 0x91, 0x8B10, 0x8001
    position = 123 | (321 << 16) | (5 << 27) | (2 << 30)
    entry = struct.pack("<IHBBIHH16H", 0x40020000, 0xB10, 0, 0, position, 5, 3, *scripts)
    # Two 4bpp pixel words and a sixteen-entry palette keep every section aligned.
    palette = struct.pack("<IHHHH", 44, 0, 480, 16, 1) + bytes(32)
    pixels = struct.pack("<IHHHH", 16, 0, 0, 2, 1) + bytes([0x10, 0x32, 0x54, 0x76])
    tim = struct.pack("<II", 0x10, 8) + palette + pixels
    sections = [
        struct.pack("<I", 1) + entry, events, b"txt\0", bytes(4),
        b"recs", b"acts", b"geom", struct.pack("<I", 4) + tim,
        struct.pack("<I", 0), struct.pack("<II", 1, 0x12345678),
    ]
    offsets = []
    cursor = 40
    for section in sections:
        offsets.append(cursor)
        cursor += len(section)
    return struct.pack("<10I", *offsets) + b"".join(sections)


class FieldSceneTests(unittest.TestCase):
    """Exercise decoded fields, lossless output and rejection of bad input."""

    def test_decodes_chest_without_external_game_files(self):
        """@brief Decode packed bit fields and a recognized chest."""
        scene = FieldScene.parse(make_scene())
        entry = scene.layout[0]
        self.assertEqual(entry["position_bits"], {
            "x": 123, "z": 321, "unknown_bits_27_29": 5, "y": 2,
        })
        self.assertTrue(entry["control"]["hidden"])
        chest = entry["common_chest"]
        self.assertEqual(chest["item_id"], 0x91)
        self.assertEqual(chest["collection_variable"], 0xB10)
        self.assertTrue(chest["alternate_facing"])
        self.assertEqual(chest["initializer_file_offset"], scene.sections[1].offset + 4)
        self.assertEqual(scene.group_bounds_count, 1)

    def test_does_not_assign_reward_to_unknown_chest_script(self):
        """@brief Keep a graphics candidate distinct from a recognized reward."""
        scene = FieldScene.parse(make_scene(chest=False))
        self.assertTrue(scene.layout[0]["chest_graphics_candidate"])
        self.assertIsNone(scene.layout[0]["common_chest"])

    def test_invalid_initializer_reference_does_not_invent_reward(self):
        """@brief Preserve raw records when an initializer cannot be resolved."""
        for reference in (0xFFFF, 1, 0xFFFE):
            data = bytearray(make_scene())
            struct.pack_into("<H", data, 40 + 4 + 46, reference)
            with self.subTest(reference=reference):
                self.assertIsNone(FieldScene.parse(bytes(data)).layout[0]["common_chest"])

    def test_extract_preserves_every_byte_and_refuses_overwrite(self):
        """@brief Reassemble extracted sections and check independent TIM output."""
        source = make_scene()
        scene = FieldScene.parse(source)
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "scene"
            scene.extract(output, "synthetic.IMG")
            manifest = json.loads((output / "manifest.json").read_text())
            rebuilt = (output / manifest["header_file"]).read_bytes()
            for section in manifest["sections"]:
                payload = (output / section["file"]).read_bytes()
                self.assertEqual(len(rebuilt), section["offset"])
                self.assertEqual(len(payload), section["size"])
                rebuilt += payload
            self.assertEqual(rebuilt, source)
            texture = manifest["textures"][0]
            self.assertEqual(
                (output / texture["file"]).read_bytes(),
                source[texture["offset"]:texture["offset"] + texture["size"]],
            )
            with self.assertRaises(FileExistsError):
                scene.extract(output, "synthetic.IMG")
            self.assertEqual(
                (output / "manifest.json").read_text(),
                json.dumps(manifest, indent=2) + "\n",
            )

    def test_rejects_invalid_header_offsets_and_counts(self):
        """@brief Reject truncated, reversed, unaligned and oversized ranges."""
        source = make_scene()
        variants = [source[:39], source[:-1]]
        offsets = struct.unpack_from("<10I", source)
        for offset, value in ((0, 44), (4, len(source) + 4), (8, 41), (8, 40), (40, 0xFFFFFFFF)):
            data = bytearray(source)
            struct.pack_into("<I", data, offset, value)
            variants.append(bytes(data))
        for section_index in (8, 9):
            data = bytearray(source)
            struct.pack_into("<I", data, offsets[section_index], 0xFFFFFFFF)
            variants.append(bytes(data))
        for data in variants:
            with self.subTest(data=data[:44]):
                with self.assertRaises(SceneFormatError):
                    FieldScene.parse(data)

    def test_rejects_bad_texture_table_and_tim(self):
        """@brief Reject invalid image references and malformed TIM payloads."""
        source = make_scene()
        image_offset = struct.unpack_from("<I", source, 28)[0]
        for relative_offset, value in ((0, 0), (0, 5), (0, 0xFFFFFFFC), (4, 0x99), (12, 4)):
            data = bytearray(source)
            struct.pack_into("<I", data, image_offset + relative_offset, value)
            with self.subTest(relative_offset=relative_offset, value=value):
                with self.assertRaises(SceneFormatError):
                    FieldScene.parse(bytes(data))

    def test_cli_invalid_input_creates_no_output(self):
        """@brief Fail malformed input cleanly through the package entry point."""
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "bad.IMG"
            source.write_bytes(b"not a scene")
            output = Path(directory) / "result"
            result = subprocess.run(
                [sys.executable, "-m", "tools.scenes.field_scene",
                 "extract", str(source), str(output)],
                cwd=ROOT, capture_output=True, text=True,
            )
            self.assertEqual(result.returncode, 1)
            self.assertIn("truncated scene header", result.stderr)
            self.assertNotIn("Traceback", result.stderr)
            self.assertFalse(output.exists())

    def test_cli_extract_and_info(self):
        """@brief Exercise package imports, JSON output and successful extraction."""
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "synthetic.IMG"
            source.write_bytes(make_scene())
            output = Path(directory) / "scene"
            command = [sys.executable, "-m", "tools.scenes.field_scene"]
            info = subprocess.run(
                command + ["info", str(source), "--json"],
                cwd=ROOT, capture_output=True, text=True, check=True,
            )
            subprocess.run(
                command + ["extract", str(source), str(output)],
                cwd=ROOT, capture_output=True, text=True, check=True,
            )
            self.assertEqual(
                json.loads(info.stdout),
                json.loads((output / "manifest.json").read_text()),
            )
            self.assertEqual(source.read_bytes(), make_scene())


if __name__ == "__main__":
    unittest.main()
