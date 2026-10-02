"""Complete scene exports, byte recovery and failure cleanup."""

import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.scenes.field_scene import COMMON_CHEST_INITIALIZER, SceneHeader, extract, extract_all, reconstruct
from tools.scenes.tests.test_identify_img import make_scene
from tools.scenes.tests.test_scene_resources import battle_resource


def sample_scene():
    """Build a scene with an ordinary actor, a chest and a battle resource."""
    original = make_scene()
    header = SceneHeader.parse(original)
    sections = [original[section.start:section.end] for section in header.sections(len(original))]
    chest = bytearray(48)
    struct.pack_into("<H", chest, 4, 0xBC0)
    struct.pack_into("<I", chest, 8, 395 | (180 << 16))
    struct.pack_into("<H", chest, 12, 5)
    struct.pack_into("<HH", chest, 0x18, 0x96, 0x8BC0)
    struct.pack_into("<H", chest, 0x2E, 0x8000)
    sections[0] = struct.pack("<I", 2) + bytes(48) + chest
    sections[1] = struct.pack("<H", 2) + COMMON_CHEST_INITIALIZER
    sections[1] += bytes(-len(sections[1]) % 4)
    sections[4] = struct.pack("<I", 4) + battle_resource()
    offsets = []
    offset = 40
    for section in sections:
        offsets.append(offset)
        offset += len(section)
    return struct.pack("<10I", *offsets) + b"".join(sections)


class SceneExportTests(unittest.TestCase):
    def test_full_export_preserves_bytes_and_decodes_records(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "sample.IMG"
            original = sample_scene()
            source.write_bytes(original)
            output = root / "export"
            extract(source, output)
            self.assertEqual(reconstruct(output), original)
            scene = yaml.safe_load((output / "scene.yaml").read_text())
            self.assertEqual(len(scene["sections"]), 10)
            layout = yaml.safe_load((output / "layout/000.yaml").read_text())
            self.assertNotIn("item_id", layout)
            chest = yaml.safe_load((output / "chests/001.yaml").read_text())
            self.assertEqual(chest["item_id"], 0x96)
            self.assertEqual(chest["collection_variable_ref"], 0xBC0)
            self.assertTrue(chest["alternate_facing"])
            battle = yaml.safe_load((output / "records/000.yaml").read_text())
            self.assertEqual(battle["battle"]["monsters"][0]["id"], 7)
            self.assertTrue((output / "event_scripts").is_dir())
            self.assertFalse((output / "unknown").exists())
            self.assertEqual(source.read_bytes(), original)

    def test_failed_write_or_reconstruction_leaves_no_export(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "sample.IMG"
            source.write_bytes(sample_scene())
            output = root / "export"
            with patch("tools.scenes.field_scene.write_yaml", side_effect=OSError("disk full")):
                with self.assertRaisesRegex(OSError, "disk full"):
                    extract(source, output)
            with patch("tools.scenes.field_scene.reconstruct", return_value=b"wrong bytes"):
                with self.assertRaisesRegex(ValueError, "does not reconstruct"):
                    extract(source, output)
            self.assertEqual(list(root.iterdir()), [source])

    def test_existing_destination_is_not_changed(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "sample.IMG"
            source.write_bytes(sample_scene())
            output = root / "export"
            output.mkdir()
            marker = output / "keep.txt"
            marker.write_text("keep this")
            with self.assertRaises(FileExistsError):
                extract(source, output)
            self.assertEqual(marker.read_text(), "keep this")

    def test_modified_binary_fails_byte_map_size_check(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "sample.IMG"
            source.write_bytes(sample_scene())
            output = root / "export"
            extract(source, output)
            (output / "records/000.bin").write_bytes(b"truncated")
            with self.assertRaisesRegex(ValueError, "byte map range"):
                reconstruct(output)

    def test_batch_includes_mapinfo_and_preflights_destinations(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            ana = root / "ANA"
            for group in ("INFO_TEST", "MAPINFO", "BTL_OBJ"):
                folder = ana / group
                folder.mkdir(parents=True)
                (folder / "sample.img").write_bytes(sample_scene())
            output = root / "export"
            self.assertEqual(extract_all(ana, output), 2)
            self.assertTrue((output / "MAPINFO/sample/scene.yaml").exists())
            self.assertFalse((output / "BTL_OBJ").exists())
            (ana / "INFO_TEST/another.IMG").write_bytes(sample_scene())
            with self.assertRaises(FileExistsError):
                extract_all(ana, output)
            self.assertFalse((output / "INFO_TEST/another").exists())


if __name__ == "__main__":
    unittest.main()
