"""Complete scene exports, byte recovery and failure cleanup."""

import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.data.scenes.field_scene import (COMMON_CHEST_INITIALIZER, SceneHeader, chest_catalogue, extract,
                                          extract_all, reconstruct)
from tools.data.scenes.tests.test_identify_img import make_scene
from tools.data.scenes.tests.test_scene_resources import battle_resource


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
            self.assertEqual(chest["chest_id"], 0xBC0)
            # The fixture has no step handing over 0xE040, so it is not loot.
            self.assertEqual(chest["chest_kind"], "quest")
            self.assertNotIn("also_placed_in", chest)
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
            with patch("tools.data.scenes.field_scene.write_yaml", side_effect=OSError("disk full")):
                with self.assertRaisesRegex(OSError, "disk full"):
                    extract(source, output)
            with patch("tools.data.scenes.field_scene.reconstruct", return_value=b"wrong bytes"):
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
            summary = extract_all(ana, output)
            self.assertEqual(summary.scenes, 2)
            self.assertTrue((output / "MAPINFO/sample/scene.yaml").exists())
            self.assertFalse((output / "BTL_OBJ").exists())
            (ana / "INFO_TEST/another.IMG").write_bytes(sample_scene())
            with self.assertRaises(FileExistsError):
                extract_all(ana, output)
            self.assertFalse((output / "INFO_TEST/another").exists())

    def test_batch_links_placements_of_one_chest(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            ana = root / "ANA"
            for group in ("INFO_TEST", "MAPINFO"):
                folder = ana / group
                folder.mkdir(parents=True)
                (folder / "sample.IMG").write_bytes(sample_scene())
            output = root / "export"
            summary = extract_all(ana, output)
            self.assertEqual((summary.chests, summary.quest, summary.placements, summary.chest_scenes), (1, 1, 2, 2))
            chest = yaml.safe_load((output / "INFO_TEST/sample/chests/001.yaml").read_text())
            self.assertEqual(chest["also_placed_in"], ["MAPINFO/sample/chests/001.yaml"])
            report = yaml.safe_load((output / "MAPINFO/sample/objects.yaml").read_text())
            self.assertEqual(report["objects"][1]["chest"]["also_placed_in"], ["INFO_TEST/sample/chests/001.yaml"])
            self.assertIn("Also placed in: INFO_TEST/sample/chests/001.yaml",
                          (output / "MAPINFO/sample/objects.txt").read_text())
            catalogue = yaml.safe_load((output / "chests.yaml").read_text())
            self.assertEqual(catalogue["summary"], {"chests": 1, "loot": 0, "quest": 1, "placements": 2, "scenes": 2})
            entry = catalogue["chests"][0]
            self.assertEqual((entry["chest_id"], entry["kind"], entry["item_id"]), (0xBC0, "quest", 0x96))
            self.assertNotIn("items_differ", entry)
            self.assertEqual([placement["file"] for placement in entry["placements"]],
                             ["INFO_TEST/sample/chests/001.yaml", "MAPINFO/sample/chests/001.yaml"])
            other = root / "other"
            other.mkdir()
            (other / "chests.yaml").write_text("keep")
            with self.assertRaisesRegex(FileExistsError, "chests.yaml"):
                extract_all(ana, other)
            self.assertEqual(sorted(path.name for path in other.iterdir()), ["chests.yaml"])

    def test_catalogue_marks_placements_with_different_items(self):
        placements = [
            {"file": "A/S0/chests/002.yaml", "chest_id": 0xB04, "kind": "loot", "item_id": 0xE5, "item": "a", "x": 1, "z": 2},
            {"file": "A/S1/chests/002.yaml", "chest_id": 0xB04, "kind": "loot", "item_id": 0x90, "item": "b", "x": 1, "z": 2},
        ]
        catalogue = chest_catalogue(placements)
        entry = catalogue["chests"][0]
        self.assertTrue(entry["items_differ"])
        self.assertNotIn("item_id", entry)
        self.assertEqual([placement["item_id"] for placement in entry["placements"]], [0xE5, 0x90])
        self.assertEqual(catalogue["summary"]["chests"], 1)


class BatchProgressTests(unittest.TestCase):
    def run_batch(self, source, output, *options):
        return subprocess.run(
            [sys.executable, "-m", "tools.data.scenes.field_scene", "--all",
             str(source), str(output), *options],
            cwd=Path(__file__).resolve().parents[4], capture_output=True, text=True,
        )

    def test_progress_counts_verified_exports_and_identifies_groups(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            ana = root / "ANA"
            original = sample_scene()
            for group in ("INFO_TEST", "MAPINFO"):
                folder = ana / group
                folder.mkdir(parents=True)
                (folder / "sample.IMG").write_bytes(original)
            output = root / "export"
            result = self.run_batch(ana, output)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("[1/2] Extracting INFO_TEST/sample.IMG ... done", result.stderr)
            self.assertIn("[2/2] Extracting MAPINFO/sample.IMG ... done", result.stderr)
            self.assertEqual(result.stderr.count("... done"), 2)
            self.assertIn("elapsed ", result.stderr)
            self.assertEqual(result.stdout, f"Extracted 2 scenes to {output}\n"
                             f"Chests: 1 (0 loot, 1 quest) in 2 placements across 2 scenes; "
                             f"see {output / 'chests.yaml'}\n")
            for group in ("INFO_TEST", "MAPINFO"):
                self.assertEqual(reconstruct(output / group / "sample"), original)

    def test_failed_scene_is_not_reported_complete_or_published(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            ana = root / "ANA"
            group = ana / "INFO_TEST"
            group.mkdir(parents=True)
            original = sample_scene()
            (group / "a.IMG").write_bytes(original)
            (group / "b.IMG").write_bytes(b"truncated")
            output = root / "export"
            result = self.run_batch(ana, output)
            self.assertEqual(result.returncode, 1)
            self.assertIn("[1/2] Extracting INFO_TEST/a.IMG ... done", result.stderr)
            self.assertIn("[2/2] Extracting INFO_TEST/b.IMG ... failed", result.stderr)
            self.assertEqual(result.stderr.count("... done"), 1)
            self.assertIn("truncated scene header", result.stderr)
            self.assertEqual(result.stdout, "")
            self.assertEqual(reconstruct(output / "INFO_TEST/a"), original)
            self.assertFalse((output / "INFO_TEST/b").exists())

    def test_no_progress_preserves_final_summary(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            ana = root / "ANA"
            group = ana / "INFO_TEST"
            group.mkdir(parents=True)
            (group / "sample.IMG").write_bytes(sample_scene())
            output = root / "export"
            result = self.run_batch(ana, output, "--no-progress")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stderr, "")
            self.assertEqual(result.stdout, f"Extracted 1 scenes to {output}\n"
                             f"Chests: 1 (0 loot, 1 quest) in 1 placements across 1 scenes; "
                             f"see {output / 'chests.yaml'}\n")


if __name__ == "__main__":
    unittest.main()
