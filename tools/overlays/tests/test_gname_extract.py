"""Exercise GNAME's exporter with invented resources, without disc files."""

import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.assets.name_entry_resource import NameEntryResource, RecordTable
from tools.assets.psx_tim import TimBlock, TimImage
from tools.overlays import gname
from tools.overlays.tests.test_cload_extract import FakeOverlay as FakeCload
from tools.overlays.tests.test_field_extract import png_pixels

ADDRESS = 0x80142000


class FakeOverlay:
    """Small text and images with the same fixed table sizes as the game."""

    def __init__(self, version="us"):
        self.version = version
        self.symbols = {}
        self.data = bytearray()

        def mark(key):
            self.symbols[gname.SYMBOL_NAMES[key]] = ADDRESS + len(self.data)

        mark("panels")
        self.data += struct.pack("<5I", 0, 1, 2, 3, 4)
        mark("categories")
        self.data += struct.pack("<2I", 0xFF, 0)
        mark("glyphs")
        self.data += struct.pack("<4BI", 1, 0, 2, 2, 1) * 39
        mark("cursors")
        self.data += struct.pack("<HBB", 300 | (5 << 9), 12, 1) * 13
        mark("kanji_offsets")
        self.data += struct.pack("<45I", *([0] * 44), 2)
        mark("records")
        panels = (b"A\x00", b"A\x00", b"Choose\x14\x00", b"\x00\x00")
        history = (b"History\x00", b"\x00\x00")
        random = (b"Random\x00", b"More\x00\x00")
        if version == "jp":
            panels = (b"\x41\x19\x00\x00", b"\x41\x00", b"\x00", b"\x19\x00\x00\x00")
            history = random = (b"\x41\x00", b"\x19\x00\x00\x00")
        self.resource = NameEntryResource(4, (
            RecordTable(panels), RecordTable((b"\x19\x00\x00", b"\x19\x01\x00\x00")),
            RecordTable(history), RecordTable(random),
        ))
        self.data += self.resource.to_bytes()
        mark("texture")
        palettes = struct.pack("<32H", 0, 31, 992, 31744, *([32767] * 12),
                               0, 31744, 31, 992, *([32767] * 12))
        self.tim = TimImage(8, TimBlock(0, 480, 16, 2, palettes),
                           TimBlock(0, 0, 1, 2, bytes.fromhex("10 32 23 01")))
        self.data += self.tim.to_bytes()
        self.data += self.data[-4:]
        mark("layout")
        self.data += struct.pack("<Ihh", 1, -5, 20) * 20
        mark("animation")
        self.data += bytes([16, 2, 1, 3, 20, 4, 2, 0, 0, 0, 0, 0]) * 7
        if version == "us":
            self.data += bytes(4)
        mark("variables")

    def offset(self, key):
        return self.symbols[gname.SYMBOL_NAMES[key]] - ADDRESS

    def write(self, root, skip_symbol=None):
        config, assets = root / "config", root / "assets"
        if self.version == "jp":
            FakeCload("jp").write(root)
        else:
            (config / "symbols").mkdir(parents=True)
            (config / "overlays").mkdir()
            assets.mkdir()
        (assets / "gname_data.databin.bin").write_bytes(self.data)
        segment = {"start": 1, "vram": ADDRESS,
                   "subsegments": [[1, "databin", "gname_data"], [1 + len(self.data), ".bss", "gname"]]}
        (config / gname.OVERLAY_CONFIG).write_text(yaml.safe_dump({"segments": [segment, [33 + len(self.data)]]}))
        lines = [f"{name} = 0x{address:08X};" for name, address in self.symbols.items() if name != skip_symbol]
        (config / gname.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return gname.Inputs(self.version, config, assets)


class ExtractTest(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        self.output = self.root / "out"
        self.overlay = FakeOverlay()

    def load(self, file):
        return yaml.safe_load((self.output / file).read_text(encoding="utf-8"))

    def assert_no_output(self):
        self.assertFalse(self.output.exists())
        self.assertEqual(list(self.root.glob(".out-*")), [])

    def test_names_keep_indices_raw_bytes_and_dictionary_words(self):
        gname.extract(self.overlay.write(self.root), self.output)
        entries = self.load("text/panel_records.yaml")["entries"]
        self.assertEqual([entry["text"] for entry in entries], ["A", "A", "Choose the", ""])
        self.assertEqual([entry["index"] for entry in entries], [0, 1, 2, 3])
        self.assertEqual(entries[3]["bytes"], "00 00")
        kanji = self.load("text/kanji_records.yaml")["entries"]
        self.assertEqual(kanji[0]["text"], "{19 00}")
        self.assertEqual(kanji[1]["text"], "{19 01}")
        self.assertEqual((self.output / "text/resource.bin").read_bytes(), self.overlay.resource.to_bytes())

    def test_japanese_records_use_cload_and_keep_zero_second_bytes(self):
        gname.extract(FakeOverlay("jp").write(self.root), self.output)
        entries = self.load("text/panel_records.yaml")["entries"]
        self.assertEqual(entries[0]["text"], "\uff21\uff21")
        self.assertEqual(entries[0]["bytes"], "41 19 00 00")
        self.assertEqual(entries[3]["text"], "\uff21")
        self.assertEqual(self.load("byte-map.yaml")["text_decoder"]["overlay"], "CLOAD")

    def test_palettes_and_glyph_crops_use_correct_pixels(self):
        gname.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "image/image.tim").read_bytes(), self.overlay.tim.to_bytes())
        width, height, rows = png_pixels(self.output / "image/palette_00.png")
        self.assertEqual((width, height), (4, 2))
        self.assertEqual(rows[:17], bytes([0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255]))
        width, height, rows = png_pixels(self.output / "image/glyphs/00.png")
        self.assertEqual((width, height), (2, 2))
        self.assertEqual(rows, bytes([0, 0, 0, 255, 255, 255, 0, 0, 255,
                                     0, 255, 0, 0, 255, 0, 0, 255, 255]))
        self.assertEqual(len(list((self.output / "image/glyphs").glob("*.png"))), 39)

    def test_tables_decode_signed_layouts_and_packed_cursors(self):
        gname.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(self.load("tables/kanji_category_map.yaml")["entries"], [None, 0])
        cursor = self.load("tables/tab_cursor_layout.yaml")["entries"][0]
        self.assertEqual(cursor, {"x": 300, "sprite_index": 5, "y": 12, "glyph_id": 1})
        sprite = self.load("tables/layout_sprite_sequence.yaml")["sprites"][0]
        self.assertEqual(sprite, {"glyph_id": 1, "x": -5, "y": 20})
        animation = self.load("tables/glyph_append_animation.yaml")
        self.assertEqual(animation["frame_count"], 7)
        self.assertEqual(animation["frames"][0]["sprites"][0]["control"], 3)

    def test_byte_map_covers_the_blob_and_keeps_unidentified_bytes(self):
        self.overlay.data[-4:] = b"test"
        gname.extract(self.overlay.write(self.root), self.output)
        cursor = 0
        for part in self.load("byte-map.yaml")["ranges"]:
            self.assertEqual(int(part["offset"], 16), cursor)
            cursor += int(part["size"], 16)
        self.assertEqual(cursor, len(self.overlay.data))
        self.assertEqual((self.output / f"unknown/{ADDRESS + cursor - 4:08X}.bin").read_bytes(), b"test")

    def test_missing_inputs_report_the_required_files(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_name_entry_tim")
        with self.assertRaisesRegex(ValueError, "g_name_entry_tim.*SYMBOL_NAMES"):
            gname.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_and_truncated_blob_write_nothing(self):
        inputs = self.overlay.write(self.root)
        path = inputs.assets / "gname_data.databin.bin"
        path.write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            gname.extract(inputs, self.output)
        path.unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            gname.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_japanese_chart_writes_nothing(self):
        inputs = FakeOverlay("jp").write(self.root)
        (inputs.assets / "blob.databin.bin").unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            gname.extract(inputs, self.output)
        self.assert_no_output()

    def test_bss_cannot_be_absorbed_into_the_blob(self):
        self.overlay.symbols[gname.SYMBOL_NAMES["variables"]] += 4
        with self.assertRaisesRegex(ValueError, "must end where g_custom_name_buf starts"):
            gname.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_invalid_resource_offsets_write_nothing(self):
        struct.pack_into("<I", self.overlay.data, self.overlay.offset("records") + 4, 0xFFFF)
        with self.assertRaisesRegex(ValueError, "first table offset"):
            gname.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_zero_second_byte_still_requires_a_terminator(self):
        with self.assertRaisesRegex(ValueError, "no string terminator"):
            gname.decode_record(b"\x19\x00", "us", "kanji_records", None)

    def test_bad_glyph_bounds_or_palette_write_nothing(self):
        inputs = self.overlay.write(self.root)
        path = inputs.assets / "gname_data.databin.bin"
        self.overlay.data[self.overlay.offset("glyphs")] = 255
        path.write_bytes(self.overlay.data)
        with self.assertRaisesRegex(ValueError, "glyph 0 lies outside"):
            gname.extract(inputs, self.output)
        self.overlay.data[self.overlay.offset("glyphs")] = 1
        struct.pack_into("<I", self.overlay.data, self.overlay.offset("glyphs") + 4, 99)
        path.write_bytes(self.overlay.data)
        with self.assertRaisesRegex(ValueError, "unknown palette"):
            gname.extract(inputs, self.output)
        self.assert_no_output()

    def test_tims_trailing_word_must_really_be_a_copy(self):
        self.overlay.data[self.overlay.offset("layout") - 1] ^= 1
        with self.assertRaisesRegex(ValueError, "does not duplicate"):
            gname.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_record_boundaries_must_match_the_archive(self):
        struct.pack_into("<I", self.overlay.data, 16, 5)
        with self.assertRaisesRegex(ValueError, "final boundary differs"):
            gname.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_write_failure_removes_staging(self):
        with patch.object(gname, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                gname.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_existing_output_is_preserved(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            gname.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
