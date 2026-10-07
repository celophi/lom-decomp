"""Readable exports preserve values and make uncertain script boundaries visible."""

import io
from pathlib import Path
import struct
import tempfile
import unittest
from contextlib import redirect_stdout

import yaml

from tools.data.scenes.actor_scripts import decode_script, format_disassembly, main, read_actor_scripts
from tools.data.scenes.field_scene import extract, reconstruct
from tools.data.scenes.presentation import dump_yaml
from tools.data.scenes.scene_format import SceneHeader
from tools.data.scenes.tests.test_actor_sections import replace_section
from tools.data.scenes.tests.test_field_scene import sample_scene
from tools.data.scenes.tests.test_identify_img import make_scene


class PresentationTests(unittest.TestCase):
    def test_hex_scalars_compact_lists_and_sentinels_keep_numeric_values(self):
        document = {
            "offset": "0x1234", "flags": "0x0080", "raise_mask": 69,
            "resource_id": 1, "size": 32, "hp_base": 25, "hp_growth": 0xFFFF,
            "animation": 0xFFFF, "image_slots": [0, 255, 4], "relative_offsets": [16, 24],
            "stats": [{"base": 5, "growth": 6}],
        }
        text = dump_yaml(document)
        decoded = yaml.safe_load(text)
        self.assertEqual(decoded, {**document, "offset": 0x1234, "flags": 0x80})
        self.assertIn("offset: 0x00001234", text)
        self.assertIn("flags: 0x0080", text)
        self.assertIn("raise_mask: 0x45", text)
        self.assertIn("hp_base: 25", text)
        self.assertIn("hp_growth: 0xFFFF", text)
        self.assertIn("image_slots: [0, 0xFF, 4]", text)
        self.assertIn("{base: 5, growth: 6}", text)
        self.assertEqual(document["offset"], "0x1234")

    def test_omission_preserves_zeroes_false_flags_and_meaningful_empty_lists(self):
        document = {"remaining_bytes": "", "trailing_bytes": "00", "directory_trailing_bytes": "",
                    "operands": {}, "reward_indices": [], "index": 0, "instrument": False}
        decoded = yaml.safe_load(dump_yaml(document))
        self.assertEqual(decoded, {"trailing_bytes": "00", "reward_indices": [], "index": 0,
                                   "instrument": False})

    def test_multiline_raw_bytes_round_trip_without_truncation(self):
        raw = bytes(range(64))
        text = dump_yaml({"item_bytes": raw.hex(" ")})
        self.assertIn("item_bytes: |-", text)
        self.assertEqual(bytes.fromhex(yaml.safe_load(text)["item_bytes"]), raw)
        actions = [raw[:8].hex(" "), raw[8:16].hex(" ")]
        text = dump_yaml({"actions": actions})
        self.assertIn("actions:\n  - ", text)
        self.assertEqual(yaml.safe_load(text)["actions"], actions)

    def test_listing_shows_aliases_empty_entries_unresolved_offsets_and_fallthrough(self):
        raw = struct.pack("<6H", 12, 13, 12, 2, 16, 99) + bytes([0xA3, 0xFF, 0, 0])
        scene = replace_section(make_scene(), "actor_scripts", raw)
        document = read_actor_scripts(scene, SceneHeader.parse(scene))
        listing = format_disassembly(document)
        self.assertIn("SCRIPT 0   Entries: 0, 2", listing)
        self.assertIn("FALLTHROUGH -> SCRIPT 1", listing)
        self.assertEqual(listing.count("UNRESOLVED:"), 2)
        self.assertIn("EMPTY (section end)", listing)
        self.assertIn("Remaining bytes (not decoded):", listing)
        self.assertIn("00 00", listing)

    def test_listing_reports_unknown_truncated_and_unterminated_ranges(self):
        for raw, reason in ((b"\x7f\xff", "UNKNOWN_OPCODE"), (b"\xa9\x12", "TRUNCATED_OPERANDS"),
                            (b"\xa3", "RANGE_END")):
            with self.subTest(reason=reason):
                script = {"index": 0, **decode_script(raw, 0x100)}
                listing = format_disassembly({"offset": "0xFE", "size": len(raw) + 2,
                                              "entries": [{"index": 0, "relative_offset": "0x2", "script": 0}],
                                              "scripts": [script]})
                self.assertIn(f"[STOP: {reason}]", listing)
                self.assertNotIn("[END]", listing)
                if reason != "RANGE_END":
                    self.assertIn(raw.hex(" ").upper(), listing)

    def test_scene_overview_and_cli_listing_agree_with_integrated_export(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "sample.IMG"
            source.write_bytes(sample_scene())
            output = root / "export"
            extract(source, output)
            overview = (output / "scene.txt").read_text()
            self.assertIn("chests/001.yaml", overview)
            self.assertIn("records/000.yaml  template 0, ID 7: Test monster", overview)
            self.assertIn("Editing them does not rebuild the IMG", overview)
            text = io.StringIO()
            with redirect_stdout(text):
                self.assertEqual(main([str(source), "--format", "text"]), 0)
            self.assertEqual(text.getvalue(), (output / "actor_scripts/data.txt").read_text())
            byte_map = yaml.safe_load((output / "byte-map.yaml").read_text())
            script = next(entry for entry in byte_map["sections"] if entry["name"] == "actor_scripts")
            self.assertEqual(script["listing"], "actor_scripts/data.txt")
            self.assertEqual(reconstruct(output), source.read_bytes())


if __name__ == "__main__":
    unittest.main()
