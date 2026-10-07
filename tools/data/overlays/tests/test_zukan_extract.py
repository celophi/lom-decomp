"""Check ZUKAN's resource boundaries with synthetic data and no disc files."""

import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.data.formats.psx_tim import TimBlock, TimImage
from tools.data.overlays import zukan
from tools.data.overlays.tests.test_cload_extract import FakeOverlay as FakeCload
from tools.data.overlays.tests.test_field_extract import png_pixels
from tools.data.overlays.tests.test_overlay_tools import make_table

ADDRESS = 0x80144000
TABLE_ADDRESS = 0x80140020
PAGE_PALETTES = struct.pack("<32H", 0, 0x001F, 0x03E0, 0x7C00, *([0x7FFF] * 12), 0, 0x7C00, *([0x03E0] * 14))
BORDER_PALETTE = struct.pack("<16H", 0, *([0x001F] * 15))
ENTRY_COUNT = 1018


def sprite_record(draw_mode, u, v, palette, width, height, x, y):
    return struct.pack("<IIHH", draw_mode | u << 8, v | palette << 8 | width << 14 | height << 23, x, y)


class FakeOverlay:
    """A four-section archive, 21 sprites and the four entry tables at their C lengths."""

    def __init__(self, version="us"):
        self.version = version
        self.symbols = {}
        self.page = TimImage(8, TimBlock(0, 480, 32, 1, PAGE_PALETTES), TimBlock(0, 0, 2, 2, bytes.fromhex("10 32 21 03 32 10 03 21")))
        self.border = TimImage(8, TimBlock(0, 480, 16, 1, BORDER_PALETTE), TimBlock(0, 0, 1, 2, bytes.fromhex("12 21 21 12")))
        if version == "us":
            names = [b"----", b"Home", b"Domina"] + [b"Entry"] * 10
            categories = [b""] + [f"Category {i}".encode() for i in range(1, 12)]
        else:
            names = [b"\x41\x19\x00", b"\x41"] + [b"\x41"] * 11
            categories = [b"\x41"] * 12
        self.names = make_table(names)
        self.names += bytes(-len(self.names) % 4)
        self.categories = make_table(categories)
        sections = [self.page.to_bytes(), self.border.to_bytes(), self.names, self.categories]
        offsets = []
        position = 20
        for section in sections:
            offsets.append(position)
            position += len(section)
        self.archive = struct.pack("<5I", 4, *offsets) + b"".join(sections)
        data = bytearray(self.archive)
        self.tail_start = len(data)
        data += bytes(-len(data) % 4 + 4)
        self.symbols[zukan.SYMBOL_NAMES["archive"]] = ADDRESS
        self.symbols[zukan.SYMBOL_NAMES["sprites"]] = ADDRESS + len(data)
        for index in range(zukan.SPRITE_COUNT):
            data += sprite_record(index % 2, 0, 0, index % 2, 4, 2, index, 200)
        data += bytes(4)
        self.symbols[zukan.SYMBOL_NAMES["variables"]] = ADDRESS + len(data)
        data += bytes(64)
        self.data = data

        self.ranges = struct.pack("<13i", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13)
        self.groups = struct.pack("<7i", 0, 1, 2, 3, 4, 5, 6)
        self.values = struct.pack(f"<{ENTRY_COUNT}h", *[0 if i % 5 == 0 else i for i in range(ENTRY_COUNT)])
        order = [2, 1, 0x400] + [0] * 716
        self.order = struct.pack("<719H", *order)
        tables = self.ranges + self.groups + self.values + self.order + bytes(2)
        self.symbols[zukan.SYMBOL_NAMES["category_ranges"]] = TABLE_ADDRESS
        self.symbols[zukan.SYMBOL_NAMES["group_ranges"]] = TABLE_ADDRESS + 52
        self.symbols[zukan.SYMBOL_NAMES["entry_values"]] = TABLE_ADDRESS + 80
        self.symbols[zukan.SYMBOL_NAMES["display_order"]] = TABLE_ADDRESS + 80 + ENTRY_COUNT * 2
        self.tables = bytearray(tables)

    def write(self, root, skip_symbol=None):
        config, assets = root / "config", root / "assets"
        if self.version == "jp":
            FakeCload("jp").write(root)
        else:
            (config / "symbols").mkdir(parents=True)
            (config / "overlays").mkdir()
            assets.mkdir()
        (assets / "zukan_data.databin.bin").write_bytes(self.data)
        (assets / "zukan_entry_tables.rodatabin.bin").write_bytes(self.tables)
        table_start = TABLE_ADDRESS - 0x80140000 + 1
        start = ADDRESS - 0x80140000 + 1
        segment = {"start": 1, "vram": 0x80140000, "subsegments": [
            [table_start, "rodatabin", "zukan_entry_tables"], [table_start + len(self.tables), "rodatabin", "zukan_run_code"],
            [table_start + len(self.tables) + 16, "c", "zukan"], [start, "databin", "zukan_data"],
        ]}
        (config / zukan.OVERLAY_CONFIG).write_text(yaml.safe_dump({"segments": [segment, [start + len(self.data)]]}))
        lines = [f"{name} = 0x{address:08X};" for name, address in self.symbols.items() if name != skip_symbol]
        (config / zukan.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return zukan.Inputs(self.version, config, assets)


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

    def test_byte_maps_cover_data_and_tables_once(self):
        zukan.extract(self.overlay.write(self.root), self.output)
        mapping = self.load("byte-map.yaml")
        for document, expected in [(mapping, len(self.overlay.data)), (mapping["rodata"][0], len(self.overlay.tables))]:
            cursor = 0
            for part in document["ranges"]:
                self.assertEqual(int(part["offset"], 16), cursor)
                cursor += int(part["size"], 16)
                if "file" in part:
                    self.assertTrue((self.output / part["file"]).is_file())
            self.assertEqual(cursor, expected)
        self.assertEqual(mapping["ranges"][-1]["name"], "screen state")
        self.assertNotIn("file", mapping["ranges"][-1])
        self.assertFalse((self.output / "unknown").exists())

    def test_archive_textures_keep_the_original_tims(self):
        zukan.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "images/page/image.tim").read_bytes(), self.overlay.page.to_bytes())
        self.assertEqual((self.output / "images/border/image.tim").read_bytes(), self.overlay.border.to_bytes())
        sections = self.load("archive/archive.yaml")["sections"]
        self.assertEqual([section["name"] for section in sections], list(zukan.ARCHIVE_SECTIONS))
        border = self.load("images/border/image.yaml")
        self.assertEqual(border["upload"], {"pixels": [832, 256], "clut": [0, 498]})
        self.assertEqual([preview["file"] for preview in border["previews"]], ["palette_00.png", "palette_01.png"])

    def test_sprites_use_the_page_palettes_on_both_textures(self):
        zukan.extract(self.overlay.write(self.root), self.output)
        sprites = self.load("images/sprites/sprites.yaml")["entries"]
        self.assertEqual(len(sprites), zukan.SPRITE_COUNT)
        self.assertEqual({key: sprites[1][key] for key in ("texture", "u", "v", "width", "height", "palette", "x", "y")},
                         {"texture": "border", "u": 0, "v": 0, "width": 4, "height": 2, "palette": 1, "x": 1, "y": 200})
        width, height, rows = png_pixels(self.output / "images/sprites/00.png")
        self.assertEqual((width, height), (4, 2))
        # A filter byte, then page pixels 0, 1, 2, 3 through page palette 0: transparent, red, green, blue.
        self.assertEqual(rows[:17], bytes([0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255]))
        width, height, rows = png_pixels(self.output / "images/sprites/01.png")
        self.assertEqual((width, height), (4, 2))
        # Border pixels 2, 1 through page palette 1 (green, blue), not the border's own red.
        self.assertEqual(rows[:9], bytes([0, 0, 255, 0, 255, 0, 0, 255, 255]))

    def test_text_and_tables_are_named(self):
        zukan.extract(self.overlay.write(self.root), self.output)
        names = self.load("text/entry_names.yaml")["entries"]
        self.assertEqual([entry["text"] for entry in names[:3]], ["----", "Home", "Domina"])
        categories = self.load("tables/categories.yaml")["entries"]
        self.assertEqual(len(categories), 12)
        self.assertEqual(categories[1], {"index": 1, "name": "Category 1", "first_entry": 1, "stored_end": 2,
                                         "list_end": 2, "first_name": "Home"})
        self.assertEqual(categories[7]["list_end"], 0x1C6)
        self.assertEqual(categories[8]["list_end"], 0x240)
        self.assertEqual(categories[11]["extra_entries"], [0x1C6, 0x1C6 + 0x1A])
        entries = self.load("tables/entries.yaml")["entries"]
        self.assertEqual(len(entries), ENTRY_COUNT)
        self.assertEqual(entries[2], {"index": 2, "name": "Domina", "category": 2, "value": 2, "cd_resource": "0xBFE"})
        self.assertNotIn("cd_resource", entries[5])
        self.assertIsNone(entries[20]["name"])
        order = self.load("tables/display_order.yaml")
        self.assertEqual([entry["entry"] for entry in order["entries"]], [2, 1])
        self.assertEqual(len(order["trailing_values"]), 716)
        groups = self.load("tables/history_groups.yaml")["entries"]
        self.assertEqual(groups[5], {"index": 5, "first_entry": 0x1E5, "end_entry": 0x1E6, "first_name": None,
                                     "unlock_bit": "0x127"})

    def test_japanese_text_reads_cload_and_keeps_zero_second_bytes(self):
        zukan.extract(FakeOverlay("jp").write(self.root), self.output)
        entry = self.load("text/entry_names.yaml")["entries"][0]
        self.assertEqual(entry["text"], "\uff21\uff21")
        self.assertEqual(entry["bytes"], "41 19 00")
        self.assertEqual(self.load("tables/categories.yaml")["entries"][3]["name"], "\uff21")
        self.assertEqual(self.load("byte-map.yaml")["text_decoder"]["overlay"], "CLOAD")

    def test_unrecognized_archive_tail_is_preserved(self):
        self.overlay.data[self.overlay.tail_start + 1] = 0x73
        zukan.extract(self.overlay.write(self.root), self.output)
        address = ADDRESS + self.overlay.tail_start
        saved = (self.output / f"unknown/{address:08X}.bin").read_bytes()
        self.assertEqual(saved[:2], b"\x00\x73")

    def test_missing_symbol_reports_what_to_update(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_zukan_display_order")
        with self.assertRaisesRegex(ValueError, "g_zukan_display_order.*SYMBOL_NAMES"):
            zukan.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_tables_or_truncated_data_writes_nothing(self):
        inputs = self.overlay.write(self.root)
        data = inputs.assets / "zukan_data.databin.bin"
        data.write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            zukan.extract(inputs, self.output)
        data.write_bytes(self.overlay.data)
        (inputs.assets / "zukan_entry_tables.rodatabin.bin").unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            zukan.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_japanese_chart_writes_nothing(self):
        inputs = FakeOverlay("jp").write(self.root)
        (inputs.assets / "blob.databin.bin").unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            zukan.extract(inputs, self.output)
        self.assert_no_output()

    def test_bad_archive_offsets_write_nothing(self):
        struct.pack_into("<I", self.overlay.data, 16, len(self.overlay.archive) + 0x100)
        with self.assertRaisesRegex(ValueError, "section offsets"):
            zukan.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_sprite_outside_its_texture_writes_nothing(self):
        start = self.overlay.symbols[zukan.SYMBOL_NAMES["sprites"]] - ADDRESS
        self.overlay.data[start : start + 12] = sprite_record(0, 6, 0, 0, 4, 2, 0, 0)
        with self.assertRaisesRegex(ValueError, "UI sprite 0 lies outside"):
            zukan.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_display_order_without_terminator_writes_nothing(self):
        start = self.overlay.symbols[zukan.SYMBOL_NAMES["display_order"]] - TABLE_ADDRESS
        struct.pack_into("<H", self.overlay.tables, start + 4, 0)
        with self.assertRaisesRegex(ValueError, "terminator"):
            zukan.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_failed_write_removes_staging(self):
        with patch.object(zukan, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                zukan.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_existing_output_is_preserved(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            zukan.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
