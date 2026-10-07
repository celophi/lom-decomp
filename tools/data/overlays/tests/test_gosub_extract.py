"""Check GOSUB's resource boundaries with synthetic data and no disc files."""

import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.data.formats.psx_tim import TimBlock, TimImage
from tools.data.overlays import gosub
from tools.data.overlays.tests.test_cload_extract import FakeOverlay as FakeCload
from tools.data.overlays.tests.test_field_extract import png_pixels
from tools.data.overlays.tests.test_overlay_tools import make_table

ADDRESS = 0x80147000
GROUP_ADDRESS = 0x80140068
PALETTE = struct.pack("<16H", 0, 0x001F, 0x03E0, 0x7C00, *([0x7FFF] * 12))


class FakeOverlay:
    """Includes the count before the portrait symbol and the font/color overlap."""

    def __init__(self, version="us"):
        self.version = version
        self.symbols = {}
        self.data = bytearray()

        def mark(key):
            self.symbols[gosub.SYMBOL_NAMES[key]] = ADDRESS + len(self.data)

        mark("image")
        self.tim = TimImage(8, TimBlock(0, 480, 16, 1, PALETTE),
                           TimBlock(0, 0, 1, 2, bytes.fromhex("10 32 23 01")))
        self.data += self.tim.to_bytes()
        self.data += self.data[-4:]
        mark("text")
        sections = []
        for index in range(12):
            strings = [b"Name", b"Name"] if version == "us" else [b"\x41\x19\x00", b"\x41"]
            if index == 11:
                strings = [f"Color {i}".encode() for i in range(33)] if version == "us" else [b"\x41"] * 33
            sections.append(make_table(strings))
        offsets = []
        position = 52
        for section in sections:
            offsets.append(position)
            position += len(section)
        self.text = struct.pack("<13I", 12, *offsets) + b"".join(sections)
        self.data += self.text
        self.symbols[gosub.SYMBOL_NAMES["portraits"]] = ADDRESS + len(self.data) + 4
        images = [PALETTE + bytes([0x10 + i]) * 1152 for i in range(3)]
        self.portraits = struct.pack("<4I", 3, 16, 16 + 1184, 16 + 2368) + b"".join(images)
        self.data += self.portraits
        self.data += self.data[-4:]
        self.symbols[gosub.SYMBOL_NAMES["font"]] = ADDRESS + len(self.data) + 20
        self.font = TimImage(8, TimBlock(0, 480, 16, 1, PALETTE),
                            TimBlock(320, 0, 16, 16, bytes([0x21]) * 420 + bytes([0x32]) * 92))
        self.data += self.font.to_bytes()
        self.symbols[gosub.SYMBOL_NAMES["colors"]] = self.symbols[gosub.SYMBOL_NAMES["font"]] + 464
        self.data += bytes(4)  # The font ends at item_colors + 92; active colors start at +96.
        self.data += bytes(i % 33 for i in range(37)) + bytes(3)
        mark("glyphs")
        for index in range(82):
            self.data += bytes(8) if index in (0, 20, 53) else struct.pack("<4B2H", 1, 0, 0, 0, 2, 2)
        self.data += bytes(4)
        mark("bss")
        self.symbols[gosub.SYMBOL_NAMES["group_firsts"]] = GROUP_ADDRESS
        self.symbols[gosub.SYMBOL_NAMES["group_counts"]] = GROUP_ADDRESS + 12
        self.groups = struct.pack("<6i", 0, 11, 23, 11, 12, 4)

    def offset(self, key):
        return self.symbols[gosub.SYMBOL_NAMES[key]] - ADDRESS

    def write(self, root, skip_symbol=None):
        config, assets = root / "config", root / "assets"
        if self.version == "jp":
            FakeCload("jp").write(root)
        else:
            (config / "symbols").mkdir(parents=True)
            (config / "overlays").mkdir()
            assets.mkdir()
        (assets / "gosub_data.databin.bin").write_bytes(self.data)
        (assets / "gosub_groups.rodatabin.bin").write_bytes(self.groups)
        start = ADDRESS - 0x80140000 + 1
        segment = {"start": 1, "vram": 0x80140000, "subsegments": [
            [0x69, "rodatabin", "gosub_groups"], [0x81, "c", "gosub"],
            [start, "databin", "gosub_data"], [start + len(self.data), ".bss", "gosub"],
        ]}
        (config / gosub.OVERLAY_CONFIG).write_text(yaml.safe_dump({"segments": [segment, [start + len(self.data) + 32]]}))
        lines = [f"{name} = 0x{address:08X};" for name, address in self.symbols.items() if name != skip_symbol]
        (config / gosub.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return gosub.Inputs(self.version, config, assets)


class ExtractTest(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        self.output = self.root / "out"
        self.overlay = FakeOverlay()

    def load(self, name):
        return yaml.safe_load((self.output / name).read_text(encoding="utf-8"))

    def assert_no_output(self):
        self.assertFalse(self.output.exists())
        self.assertEqual(list(self.root.glob(".out-*")), [])

    def test_twelve_text_sections_keep_indices_and_message_names(self):
        gosub.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(len(self.load("text/archive.yaml")["sections"]), 12)
        entries = self.load("text/item_names.yaml")["entries"]
        self.assertEqual([entry["text"] for entry in entries], ["Name", "Name"])
        self.assertEqual([entry["index"] for entry in entries], [0, 1])
        self.assertEqual(self.load("text/messages.yaml")["entries"][0]["symbol"], "GOSUB_MSG_YES")
        self.assertEqual((self.output / "text/archive.bin").read_bytes(), self.overlay.text)

    def test_japanese_text_reads_cload_and_keeps_zero_second_bytes(self):
        gosub.extract(FakeOverlay("jp").write(self.root), self.output)
        entry = self.load("text/item_names.yaml")["entries"][0]
        self.assertEqual(entry["text"], "\uff21\uff21")
        self.assertEqual(entry["bytes"], "41 19 00")
        self.assertEqual(self.load("byte-map.yaml")["text_decoder"]["overlay"], "CLOAD")

    def test_portrait_count_is_before_the_symbol_and_outside_text(self):
        gosub.extract(self.overlay.write(self.root), self.output)
        portraits = self.load("portraits/portraits.yaml")
        self.assertEqual(int(portraits["address"], 16), self.overlay.symbols[gosub.SYMBOL_NAMES["portraits"]] - 4)
        self.assertEqual(len(portraits["entries"]), 3)
        self.assertEqual((self.output / "portraits/archive.bin").read_bytes(), self.overlay.portraits)
        width, height, rows = png_pixels(self.output / "portraits/00.png")
        self.assertEqual((width, height), (48, 48))
        self.assertEqual(rows[:9], bytes([0, 0, 0, 0, 0, 255, 0, 0, 255]))

    def test_font_tim_includes_the_overlapping_item_prefix(self):
        gosub.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "font/image.tim").read_bytes(), self.overlay.font.to_bytes())
        width, height, rows = png_pixels(self.output / "font/image.png")
        self.assertEqual((width, height), (64, 16))
        self.assertEqual(rows[-8:], bytes([0, 255, 0, 255, 0, 0, 255, 255]))
        colors = self.load("tables/item_colors.yaml")["entries"]
        self.assertEqual(len(colors), 37)
        self.assertEqual(colors[0], {"item_id": 96, "color_index": 0, "color_name": "Color 0"})
        self.assertEqual(colors[-1]["item_id"], 132)

    def test_ui_glyph_previews_keep_empty_indices(self):
        gosub.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "image/image.tim").read_bytes(), self.overlay.tim.to_bytes())
        glyphs = self.load("image/image.yaml")["glyphs"]
        self.assertEqual(len(glyphs), 82)
        self.assertNotIn("file", glyphs[0])
        self.assertEqual(len(list((self.output / "image/glyphs").glob("*.png"))), 79)
        self.assertEqual(png_pixels(self.output / "image/glyphs/01.png")[:2], (8, 2))

    def test_byte_maps_cover_data_and_rodata_once(self):
        gosub.extract(self.overlay.write(self.root), self.output)
        mapping = self.load("byte-map.yaml")
        for document, expected in [(mapping, len(self.overlay.data)), (mapping["rodata"][0], 24)]:
            cursor = 0
            for part in document["ranges"]:
                self.assertEqual(int(part["offset"], 16), cursor)
                cursor += int(part["size"], 16)
                if "file" in part:
                    self.assertTrue((self.output / part["file"]).is_file())
            self.assertEqual(cursor, expected)
        groups = self.load("tables/equipment_groups.yaml")
        self.assertEqual(groups["entries"][2], {"kind": "instrument", "first_index": 23, "count": 4})
        self.assertEqual(bytes.fromhex(groups["bytes"]), self.overlay.groups)

    def test_unrecognized_trailing_bytes_are_preserved(self):
        self.overlay.data[-4:] = b"test"
        gosub.extract(self.overlay.write(self.root), self.output)
        address = ADDRESS + len(self.overlay.data) - 4
        self.assertEqual((self.output / f"unknown/{address:08X}.bin").read_bytes(), b"test")

    def test_missing_symbol_reports_what_to_update(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_gosub_font_texture")
        with self.assertRaisesRegex(ValueError, "g_gosub_font_texture.*SYMBOL_NAMES"):
            gosub.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_rodata_or_truncated_data_writes_nothing(self):
        inputs = self.overlay.write(self.root)
        data = inputs.assets / "gosub_data.databin.bin"
        data.write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            gosub.extract(inputs, self.output)
        data.write_bytes(self.overlay.data)
        (inputs.assets / "gosub_groups.rodatabin.bin").unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            gosub.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_japanese_chart_writes_nothing(self):
        inputs = FakeOverlay("jp").write(self.root)
        (inputs.assets / "blob.databin.bin").unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            gosub.extract(inputs, self.output)
        self.assert_no_output()

    def test_invalid_portrait_offset_writes_nothing(self):
        struct.pack_into("<I", self.overlay.data, self.overlay.offset("portraits"), 0xFFFFFFFF)
        with self.assertRaisesRegex(ValueError, "icon 0 has an invalid offset"):
            gosub.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_text_section_cannot_read_portrait_bytes(self):
        struct.pack_into("<I", self.overlay.data, self.overlay.offset("text") + 48, len(self.overlay.text) + 4)
        with self.assertRaisesRegex(ValueError, "section offsets"):
            gosub.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_bad_color_index_writes_nothing(self):
        self.overlay.data[self.overlay.offset("colors") + 96] = 255
        with self.assertRaisesRegex(ValueError, "invalid color-name index"):
            gosub.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_bad_font_dimensions_write_nothing(self):
        struct.pack_into("<H", self.overlay.data, self.overlay.offset("font") + 40, 8)
        with self.assertRaises(ValueError):
            gosub.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_bss_cannot_be_absorbed_into_the_blob(self):
        self.overlay.symbols[gosub.SYMBOL_NAMES["bss"]] += 4
        with self.assertRaisesRegex(ValueError, "where BSS begins"):
            gosub.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_failed_write_removes_staging(self):
        with patch.object(gosub, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                gosub.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_existing_output_is_preserved(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            gosub.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
