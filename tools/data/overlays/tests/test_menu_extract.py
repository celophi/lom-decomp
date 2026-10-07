"""Exercise MENU's export using invented data, without game files."""

import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.data.formats.psx_tim import TimBlock, TimImage
from tools.data.overlays import menu
from tools.data.overlays.tests.test_cload_extract import FakeOverlay as FakeCload
from tools.data.overlays.tests.test_field_extract import png_pixels
from tools.data.overlays.tests.test_overlay_tools import make_table

ADDRESS = 0x80150000
RED, GREEN, BLUE, WHITE = 0x001F, 0x03E0, 0x7C00, 0x7FFF


def pixel(path, x, y):
    width, _, rows = png_pixels(path)
    start = y * (width * 4 + 1) + 1 + x * 4
    return tuple(rows[start:start + 4])


class FakeOverlay:
    """Every MENU resource at a small size, with the fixed-size texture and tables."""

    def __init__(self, version="us"):
        self.version = version
        self.symbols = {}
        data = bytearray()
        self.data = data

        def mark(key, name=None):
            self.symbols[name or menu.SYMBOL_NAMES[key]] = ADDRESS + len(data)

        def pad():
            data.extend(bytes(-len(data) % 4))

        mark("icon_sprites")
        data += bytes([0, 0, 0, 0, 2, 1, 2, 1, 0, 0, 1, 1])
        mark("icon_palettes")
        data += bytes([0x00, 0x11, 0x00])
        pad()
        mark("item_counts")
        data += bytes([2])
        mark("default_count")
        data += bytes([1, 2])
        pad()
        mark("action_codes")
        data += bytes(range(24))
        self.set_a = ADDRESS + len(data)
        data += bytes(8) + struct.pack("<HBB4s", 0x5000 | (7 << 9) | 300, 40, 9, b"\x01\x02\x03\x04")
        data += struct.pack("<HBB4s", 0x2000 | 12, 50, 3, bytes(4))
        mark("default_items")
        data += bytes(8) + struct.pack("<HBB4s", 0xF000 | 5, 6, 7, bytes(4))
        mark("text")
        self.text = self.text_resource()
        data += self.text
        mark("image")
        self.image = self.image_asset()
        data += self.image + bytes(4)
        self.labels_start = ADDRESS + len(data)
        for offset, name in zip((-menu.ACTION_LABEL_FIRST, 1, 2, 3, 4, 5), menu.LABEL_BASES):
            self.symbols[name] = self.labels_start + offset
        data += bytes([10, 11, 12, 13, 14, 15, 16, 17])
        mark("group_ids")
        data += bytes([0, 2, 1] + [0] * (menu.NODE_COUNT - 3))
        mark("content_table")
        pointers = [self.set_a, 1, self.symbols[menu.SYMBOL_NAMES["default_items"]]] + [0] * (menu.NODE_COUNT - 3)
        data += struct.pack(f"<{menu.NODE_COUNT}I", *pointers)
        self.symbols[menu.SYMBOL_NAMES["scripts"]] = ADDRESS + len(data) - menu.SCRIPT_ROW.size
        data += struct.pack("<24H", 0, 0x1000, 0x1020, 0xFFFF, *([0] * 20))
        data += bytes(menu.SCRIPT_ROW.size)
        mark("grid_sprites")
        for index in range(menu.GRID_SPRITE_COUNT):
            data += struct.pack("<HHhhhh", 0x0002, 0, index, 1, 1, 1)
        mark("cursor_icons")
        data += bytes([1, 2, 1])
        pad()
        mark("variables")
        data += bytes(32)

    def text_resource(self):
        count = menu.text_table_count()
        us = self.version == "us"
        tables = []
        for index in range(count):
            if index == 1:
                table = make_table([b"Yes\x1f\x00", b""] if us else [b"\x41\x19\x00", b""])
            elif index == 3:
                # Entry 1 points just past the table, at the next table's first byte.
                table = struct.pack("<2H", 4, 8) + b"A\x00"
            else:
                table = make_table([b"A"])
            tables.append(table + bytes(-len(table) % 4))
        offsets, cursor = [], 4 + count * 4
        for table in tables:
            offsets.append(cursor)
            cursor += len(table)
        return struct.pack(f"<{count + 1}I", count, *offsets) + b"".join(tables)

    def image_asset(self):
        first = [0] * 16 + [0, RED, GREEN, BLUE] + [WHITE] * 12 + [WHITE] * 224
        second = [0] * 16 + [0, BLUE, RED, GREEN] + [WHITE] * 12 + [WHITE] * 224
        pixels = bytearray(0x8000)
        pixels[1] = 0x21  # (2, 0) = 1, (3, 0) = 2
        pixels[129] = 0x13  # (2, 1) = 3, (3, 1) = 1
        self.tim = TimImage(8, TimBlock(0, 480, 16, 16, struct.pack("<256H", *first)),
                            TimBlock(0, 0, 64, 256, bytes(pixels)))
        tim = self.tim.to_bytes()
        return struct.pack("<3I", 2, 12, 12 + len(tim)) + tim + struct.pack("<256H", *second)

    def offset(self, name):
        return self.symbols[name] - ADDRESS

    def write(self, root, skip_symbol=None):
        config, assets = root / "config", root / "assets"
        if self.version == "jp":
            FakeCload("jp").write(root)
        else:
            (config / "symbols").mkdir(parents=True)
            (config / "overlays").mkdir()
            assets.mkdir()
        (assets / "menu_data.databin.bin").write_bytes(self.data)
        segment = {"start": 1, "vram": ADDRESS, "subsegments": [[1, "databin", "menu_data"]]}
        (config / menu.OVERLAY_CONFIG).write_text(yaml.safe_dump({"segments": [segment, [1 + len(self.data)]]}))
        lines = [f"{name} = 0x{address:08X};" for name, address in self.symbols.items() if name != skip_symbol]
        (config / menu.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return menu.Inputs(self.version, config, assets)


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

    def test_byte_map_covers_every_byte_including_runtime_variables(self):
        menu.extract(self.overlay.write(self.root), self.output)
        cursor = 0
        parts = self.load("byte-map.yaml")["ranges"]
        for part in parts:
            self.assertEqual(int(part["offset"], 16), cursor)
            cursor += int(part["size"], 16)
            if "file" in part:
                self.assertTrue((self.output / part["file"]).is_file())
        self.assertEqual(cursor, len(self.overlay.data))
        names = [part["name"] for part in parts]
        self.assertEqual(names[-1], "runtime variables")
        self.assertIn("content table", names)
        self.assertIn("input scripts", names)
        self.assertNotIn("unknown", names)
        self.assertFalse((self.output / "unknown").exists())

    def test_texture_keeps_the_asset_and_previews_both_palette_rows(self):
        menu.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "image/asset.bin").read_bytes(), self.overlay.image)
        self.assertEqual((self.output / "image/image.tim").read_bytes(), self.overlay.tim.to_bytes())
        self.assertEqual(len(list((self.output / "image").glob("palette_*.png"))), 32)
        self.assertEqual(pixel(self.output / "image/palette_01.png", 2, 0), (255, 0, 0, 255))
        self.assertEqual(pixel(self.output / "image/palette_11.png", 2, 0), (0, 0, 255, 255))
        self.assertEqual(pixel(self.output / "image/palette_01.png", 0, 0), (0, 0, 0, 0))
        image = self.load("image/image.yaml")
        self.assertEqual(image["second_palette_offset"], f"0x{12 + len(self.overlay.tim.to_bytes()):X}")

    def test_icons_are_cropped_with_their_own_palette(self):
        menu.extract(self.overlay.write(self.root), self.output)
        icons = self.load("icons/icons.yaml")
        self.assertEqual(icons["count"], 3)
        self.assertNotIn("file", icons["entries"][0])
        entry = icons["entries"][1]
        self.assertEqual((entry["u"], entry["v"], entry["width"], entry["height"]), (2, 1, 2, 1))
        self.assertEqual((entry["palette_code"], entry["clut_x"], entry["clut_y"]), ("0x11", 16, 499))
        path = self.output / "icons/001.png"
        self.assertEqual(png_pixels(path)[:2], (2, 1))
        self.assertEqual(pixel(path, 0, 0), (0, 255, 0, 255))  # index 3 through the second row
        self.assertEqual(pixel(path, 1, 0), (0, 0, 255, 255))  # index 1 through the second row

    def test_grid_preview_places_sprites_on_screen(self):
        menu.extract(self.overlay.write(self.root), self.output)
        grid = self.load("image/grid.yaml")
        self.assertEqual(len(grid["entries"]), menu.GRID_SPRITE_COUNT)
        self.assertEqual(grid["entries"][0]["palette_code"], "0x00")
        self.assertEqual(grid["entries"][menu.GRID_ALT_CLUT_START]["palette_code"], "0x01")
        path = self.output / "image/grid.png"
        self.assertEqual(png_pixels(path)[:2], (menu.GRID_SPRITE_COUNT, 2))
        self.assertEqual(pixel(path, 0, 0), (0, 0, 0, 0))  # Above every sprite.
        self.assertEqual(pixel(path, 0, 1), (0, 0, 0, 0))  # Palette 0x00, color 1 is zero.
        self.assertEqual(pixel(path, menu.GRID_ALT_CLUT_START, 1), (255, 0, 0, 255))

    def test_text_tables_keep_names_codes_and_entries_outside_their_table(self):
        menu.extract(self.overlay.write(self.root), self.output)
        resource = self.load("text/resource.yaml")
        self.assertEqual(resource["table_count"], 34)
        self.assertEqual(resource["tables"][0]["file"], "00_general.yaml")
        self.assertEqual(resource["tables"][2]["file"], "02.yaml")
        self.assertEqual((self.output / "text/resource.bin").read_bytes(), self.overlay.text)
        messages = self.load("text/01_messages.yaml")
        self.assertEqual(messages["symbol"], "MENU_TEXT_MESSAGES")
        self.assertEqual(messages["entries"][0]["text"], "Yes{1F 00}")
        self.assertEqual(messages["entries"][0]["bytes"], "59 65 73 1f 00")
        spells = self.load("text/03_spell_names.yaml")["entries"]
        self.assertNotIn("outside_table", spells[0])
        self.assertTrue(spells[1]["outside_table"])

    def test_japanese_text_uses_cload(self):
        menu.extract(FakeOverlay("jp").write(self.root), self.output)
        entry = self.load("text/01_messages.yaml")["entries"][0]
        self.assertEqual(entry["text"], "\uff21\uff21")
        self.assertEqual(entry["bytes"], "41 19 00")
        self.assertEqual(self.load("byte-map.yaml")["text_decoder"]["overlay"], "CLOAD")

    def test_content_nodes_groups_and_item_sets(self):
        menu.extract(self.overlay.write(self.root), self.output)
        nodes = self.load("content/nodes.yaml")["nodes"]
        self.assertEqual(len(nodes), menu.NODE_COUNT)
        self.assertEqual(nodes[0], {"node": 0, "group": 0, "item_count": 2, "content": f"0x{self.overlay.set_a:08X}"})
        self.assertEqual(nodes[1]["content"], "load_request")
        self.assertIsNone(nodes[3]["content"])
        counts = self.load("content/item_counts.yaml")
        self.assertEqual((counts["groups"], counts["counts"], counts["default_count_index"]), (3, [2, 1, 2], 1))
        self.assertIn("(2) items", counts["note"])
        groups = self.load("content/groups.yaml")["groups"]
        self.assertEqual(groups[2]["action_codes"], list(range(16, 24)))
        sets = self.load("content/item_sets.yaml")["sets"]
        self.assertEqual([entry["stored_items"] for entry in sets], [3, 2])
        self.assertEqual(sets[1]["symbols"], ["g_menu_default_content_items"])
        item = sets[0]["items"][1]
        self.assertEqual({key: item[key] for key in ("x", "style", "type", "type_name", "y", "action_type", "params")},
                         {"x": 300, "style": 7, "type": "0x5", "type_name": "submenu", "y": 40,
                          "action_type": 9, "params": "01 02 03 04"})
        self.assertNotIn("type_name", sets[0]["items"][2])
        self.assertEqual(sets[1]["items"][1]["type_name"], "action")

    def test_label_bases_and_scripts(self):
        menu.extract(self.overlay.write(self.root), self.output)
        labels = self.load("tables/label_ids.yaml")
        self.assertEqual(labels["values"], list(range(10, 18)))
        self.assertEqual([base["first_index"] for base in labels["bases"]], [menu.ACTION_LABEL_FIRST, -1, -2, -3, -4, -5])
        scripts = self.load("tables/input_scripts.yaml")["scripts"]
        self.assertEqual([script["script"] for script in scripts], [1, 2])
        self.assertEqual(scripts[0]["frames"], [[], ["PADLup"], ["PADRright", "PADLup"]])
        self.assertTrue(scripts[0]["terminated"])
        self.assertFalse(scripts[1]["terminated"])
        self.assertEqual(self.load("tables/cursor_icons.yaml")["frames"], [1, 2, 1])

    def test_nonzero_runtime_variables_are_preserved(self):
        self.overlay.data[-1] = 0x5A
        menu.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "unknown/variables.bin").read_bytes(), bytes(31) + b"Z")

    def test_nonzero_gap_is_saved_by_address(self):
        offset = self.overlay.offset(menu.SYMBOL_NAMES["variables"]) - 1
        self.overlay.data[offset] = 0x77
        menu.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / f"unknown/{ADDRESS + offset:08X}.bin").read_bytes(), b"\x77")

    def test_missing_symbol_reports_what_to_update(self):
        inputs = self.overlay.write(self.root, skip_symbol="D_801686B8")
        with self.assertRaisesRegex(ValueError, "D_801686B8.*LABEL_BASES"):
            menu.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_or_truncated_blob_writes_nothing(self):
        inputs = self.overlay.write(self.root)
        path = inputs.assets / "menu_data.databin.bin"
        path.write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            menu.extract(inputs, self.output)
        path.unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            menu.extract(inputs, self.output)
        self.assert_no_output()

    def test_wrong_resource_order_is_rejected(self):
        self.overlay.symbols[menu.SYMBOL_NAMES["text"]] = ADDRESS
        with self.assertRaisesRegex(ValueError, "out of resource order"):
            menu.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_text_offsets_must_stay_inside_the_resource(self):
        struct.pack_into("<I", self.overlay.data, self.overlay.offset(menu.SYMBOL_NAMES["text"]) + 8,
                         len(self.overlay.text) + 1)
        with self.assertRaisesRegex(ValueError, "text table offsets"):
            menu.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_content_pointer_outside_the_item_sets_is_rejected(self):
        struct.pack_into("<I", self.overlay.data, self.overlay.offset(menu.SYMBOL_NAMES["content_table"]),
                         self.overlay.symbols[menu.SYMBOL_NAMES["text"]])
        with self.assertRaisesRegex(ValueError, "outside the content item sets"):
            menu.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_bad_texture_header_writes_nothing(self):
        struct.pack_into("<I", self.overlay.data, self.overlay.offset(menu.SYMBOL_NAMES["image"]) + 8, 0x100)
        with self.assertRaisesRegex(ValueError, "texture header"):
            menu.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_icon_outside_the_texture_writes_nothing(self):
        self.overlay.data[self.overlay.offset(menu.SYMBOL_NAMES["icon_sprites"]) + 4] = 255
        with self.assertRaisesRegex(ValueError, "icon 1 lies outside"):
            menu.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_failed_write_removes_staging(self):
        with patch.object(menu, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                menu.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_existing_output_is_preserved(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            menu.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
