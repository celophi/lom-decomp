"""Exercise TITLE's exporter with invented resources, without disc files."""

import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.data.formats.psx_tim import TimBlock, TimImage
from tools.data.overlays import saved_game, title
from tools.data.overlays.tests.test_cload_extract import FakeOverlay as FakeCload
from tools.data.overlays.tests.test_field_extract import png_pixels

ADDRESS = 0x80052000
RED, GREEN, BLUE, WHITE = 31, 31 << 5, 31 << 10, 0x7FFF


def item(name: bytes, info: int, power: int = 0) -> bytes:
    record = bytearray(saved_game.ITEM_SIZE)
    record[: len(name)] = name
    struct.pack_into("<I", record, 0x14, info)
    struct.pack_into("<H", record, 0x24, power)
    struct.pack_into("<i", record, 0x34, 50)
    return bytes(record)


def character(name: bytes) -> bytes:
    record = bytearray(saved_game.CHARACTER_SIZE)
    record[: len(name)] = name
    record[0x18] = 0x80 | 1
    struct.pack_into("<IH", record, 0x20, (300 << 8) | 7, 48)
    struct.pack_into("<H", record, 0x30, (5 << 9) | 20)
    record[0x50:0x90] = item(b"Blade", 1 << 10, 12)
    return bytes(record)


class FakeOverlay:
    """Tiny images and records in the game's order, with the game's fixed record counts."""

    def __init__(self, version="us"):
        self.version = version
        self.symbols = {}
        self.data = bytearray()
        text = b"\x41\x19\x00" if version == "jp" else b"Knife"

        def mark(key, name=None):
            self.symbols[name or title.SYMBOL_NAMES[key]] = ADDRESS + len(self.data)

        # Two 16-color palettes; pixels 0, 1, 2, 3 on the first row, 3, 2, 1, 0 on the second.
        self.menu_tim = TimImage(8, TimBlock(0, 480, 16, 2, struct.pack("<32H", 0, RED, GREEN, BLUE, *[WHITE] * 12,
                                                                         0, BLUE, GREEN, RED, *[WHITE] * 12)),
                                 TimBlock(0, 0, 1, 2, bytes.fromhex("10 32 23 01")))
        self.backdrop = TimImage(2, None, TimBlock(0, 0, 2, 1, struct.pack("<2H", RED, 0)))
        menu = self.menu_tim.to_bytes()
        mark("menu_offsets")
        self.data += struct.pack("<3I", 2, 12, 12 + len(menu))
        mark("menu_items")
        self.data += menu
        mark("backdrop")
        self.data += self.backdrop.to_bytes() + self.backdrop.to_bytes()[-4:]
        mark("cursor_blink")
        self.data += bytes([0, 16, 32, 16])
        self.textures = []
        for index in range(title.TEXTURE_COUNT):
            self.symbols[title.TEXTURE_SYMBOL.format(index)] = ADDRESS + len(self.data)
            if index % 2:
                tim = TimImage(9, TimBlock(0, 0, 256, 1, struct.pack("<256H", 0, GREEN, *[WHITE] * 254)),
                               TimBlock(0, 0, 2, 1, bytes([1, 0, 0, 0])))
            else:
                tim = TimImage(8, TimBlock(0, 0, 16, 1, struct.pack("<16H", 0, BLUE, *[WHITE] * 14)),
                               TimBlock(0, 0, 2, 1, bytes([0x01, 0x10, 0, 0])))
            self.textures.append(tim)
            self.data += tim.to_bytes() + tim.to_bytes()[-4:]
        mark("texture_table")
        for index in range(title.TEXTURE_COUNT):
            self.data += struct.pack("<4hII", 320 + index * 64, 0, 0, 480 + index,
                                     self.symbols[title.TEXTURE_SYMBOL.format(index)], 0)
        mark("panel_uvs")
        self.data += bytes([1, 2, 3, 4, 5, 6]) * (title.WEAPON_SLOT_COUNT + 1)
        mark("sprite_uvs")
        self.data += bytes([6, 5, 4, 3, 2, 1]) * (title.WEAPON_SLOT_COUNT + 1)
        mark("layout")
        for index in range(title.LAYOUT_ENTRY_COUNT):
            self.data += struct.pack("<4B4h4HI", 3 if index == 0 else 0, index % 5, index % 11, 0,
                                     -index, 10, 0, 0, 16, 32, 64, 8, 0)
        mark("weapons")
        for index in range(title.WEAPON_SLOT_COUNT):
            self.data += item(text, index << 10, index + 1)
        mark("new_game")
        state = bytearray(title.NEW_GAME_SIZE)
        state[: len(text)] = text
        struct.pack_into("<I", state, 0x2C, 1234)
        state[0x5F0 : 0x5F0 + saved_game.CHARACTER_SIZE] = character(text)
        state[0xCE0 + 0x40 : 0xCE0 + 0x80] = item(b"Herb", 1 << 8)
        state[0x25E0 + 5] = 3
        state[-1] = 0xAB
        self.data += state
        mark("alternate")
        self.data += bytes(saved_game.LAYOUT_SIZE)
        mark("hero_default")
        self.data += character(b"Hero")
        mark("hero_continue")
        self.data += character(b"Heroine")
        mark("variables")
        self.data += bytes(0x8C)

    def offset(self, key):
        return self.symbols[title.SYMBOL_NAMES[key]] - ADDRESS

    def write(self, root, skip_symbol=None):
        config, assets = root / "config", root / "assets"
        if self.version == "jp":
            FakeCload("jp").write(root)
        else:
            (config / "symbols").mkdir(parents=True)
            (config / "overlays").mkdir()
            assets.mkdir()
        (assets / "title_data.databin.bin").write_bytes(self.data)
        segment = {"start": 1, "vram": ADDRESS, "subsegments": [[1, "databin", "title_data"]]}
        (config / title.OVERLAY_CONFIG).write_text(yaml.safe_dump({"segments": [segment, [1 + len(self.data)]]}))
        lines = [f"{name} = 0x{address:08X};" for name, address in self.symbols.items() if name != skip_symbol]
        (config / title.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return title.Inputs(self.version, config, assets)


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

    def extract_with(self, key, offset, value):
        """Change one word of the blob, then expect the export to be refused."""
        struct.pack_into("<I", self.overlay.data, self.overlay.offset(key) + offset, value)
        return self.overlay.write(self.root)

    def test_byte_map_covers_the_blob(self):
        title.extract(self.overlay.write(self.root), self.output)
        ranges = self.load("byte-map.yaml")["ranges"]
        cursor = 0
        for entry in ranges:
            self.assertEqual(int(entry["offset"], 16), cursor)
            cursor += int(entry["size"], 16)
        self.assertEqual(cursor, len(self.overlay.data))
        self.assertEqual(sum(entry["name"].endswith("trailing word") for entry in ranges), 1 + title.TEXTURE_COUNT)
        self.assertEqual(ranges[-1]["name"], "menu state")
        self.assertFalse((self.output / "unknown").exists())

    def test_nonzero_menu_state_is_kept_unchanged(self):
        self.overlay.data[-4:] = b"test"
        title.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "unknown/variables.bin").read_bytes()[-4:], b"test")

    def test_menu_images_keep_the_tim_and_show_each_palette(self):
        title.extract(self.overlay.write(self.root), self.output)
        folder = self.output / "title_menu/menu_items"
        self.assertEqual((folder / "image.tim").read_bytes(), self.overlay.menu_tim.to_bytes())
        width, height, rows = png_pixels(folder / "palette_00.png")
        self.assertEqual((width, height), (4, 2))
        self.assertEqual(rows[:17], bytes([0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255]))
        _, _, rows = png_pixels(folder / "palette_01.png")
        self.assertEqual(rows[5:9], bytes([0, 0, 255, 255]))
        image = self.load("title_menu/menu_items/image.yaml")
        self.assertEqual(image["upload"], title.MENU_UPLOADS["menu_items"])
        self.assertEqual(len(image["previews"]), 2)
        width, height, rows = png_pixels(self.output / "title_menu/backdrop/image.png")
        self.assertEqual((width, height, rows), (2, 1, bytes([0, 255, 0, 0, 255, 0, 0, 0, 0])))
        self.assertEqual(self.load("title_menu/offsets.yaml")["entries"][1]["target"], "backdrop")
        self.assertEqual(self.load("title_menu/cursor_blink.yaml")["values"], [0, 16, 32, 16])

    def test_texture_table_names_each_tim_and_its_vram_position(self):
        title.extract(self.overlay.write(self.root), self.output)
        table = self.load("save_slot_menu/textures.yaml")
        self.assertEqual(table["entry_count"], title.TEXTURE_COUNT)
        entry = table["entries"][3]
        self.assertEqual(entry["file"], "textures/03/image.yaml")
        self.assertEqual(entry["texture_vram"], {"x": 512, "y": 0})
        self.assertEqual(entry["clut_vram"], {"x": 0, "y": 483})
        image = self.load("save_slot_menu/textures/03/image.yaml")
        self.assertEqual((image["pixel_mode"], image["width"]), ("8bpp", 4))
        _, _, rows = png_pixels(self.output / "save_slot_menu/textures/03/palette_00.png")
        self.assertEqual(rows, bytes([0, 0, 255, 0, 255]) + bytes(12))
        self.assertEqual((self.output / "save_slot_menu/textures/04/image.tim").read_bytes(),
                         self.overlay.textures[4].to_bytes())

    def test_uv_and_layout_tables_decode_pixels_and_primitives(self):
        title.extract(self.overlay.write(self.root), self.output)
        uvs = self.load("save_slot_menu/panel_uvs.yaml")["entries"]
        self.assertEqual(uvs[0]["u"], 8)
        self.assertEqual(uvs[0]["origin_y"], 48)
        self.assertEqual([uvs[0]["weapon_slot"], uvs[11]["weapon_slot"]], [None, 10])
        layout = self.load("save_slot_menu/layout.yaml")["entries"]
        self.assertEqual(len(layout), title.LAYOUT_ENTRY_COUNT)
        self.assertEqual(layout[0]["flags"], {"apply_slide": True, "semi_transparent": True, "blend_mode": 0})
        self.assertEqual(layout[3]["primitive"], "panel_quad")
        self.assertEqual(layout[12]["texture"], "texture_01")
        self.assertEqual(layout[4]["position"], {"x": -4, "y": 10})

    def test_weapons_and_game_states_follow_the_saved_game_records(self):
        title.extract(self.overlay.write(self.root), self.output)
        weapons = self.load("save_slot_menu/starting_weapons.yaml")["records"]
        self.assertEqual(len(weapons), title.WEAPON_SLOT_COUNT)
        self.assertEqual(weapons[10]["name"], {"text": "Knife", "bytes": "4b 6e 69 66 65" + " 00" * 15})
        self.assertEqual((weapons[10]["weapon_category"], weapons[10]["weapon_power"]), ("bow", 11))
        state = self.load("game_state/new_game.yaml")
        self.assertEqual((state["money"], state["summary_name"]["text"]), (1234, "Knife"))
        self.assertEqual((state["size"], state["copied_size"]), (title.NEW_GAME_SIZE, title.COPIED_WORDS * 4))
        self.assertEqual(state["items"][0]["index"], 1)
        self.assertEqual(state["items"][0]["category"], "armor")
        self.assertEqual(state["item_counts"], {5: 3})
        self.assertEqual((self.output / "game_state/new_game.bin").read_bytes()[-1], 0xAB)
        hero = state["characters"][0]
        self.assertEqual((hero["character_type"], hero["pad_controlled"], hero["level"], hero["experience"]),
                         (1, True, 7, 300))
        self.assertEqual(hero["stats"][0], {"base": 20, "effective": 5})
        self.assertEqual(hero["equipment"][0]["weapon_category"], "sword")
        self.assertIsNone(hero["equipment"][1])
        hero = self.load("game_state/hero_continue.yaml")
        self.assertEqual((hero["name"]["text"], hero["hp"]), ("Heroine", 48))
        self.assertEqual((self.output / "game_state/hero_continue.bin").read_bytes(), character(b"Heroine"))

    def test_japanese_text_uses_cload_and_keeps_zero_second_bytes(self):
        title.extract(FakeOverlay("jp").write(self.root), self.output)
        weapon = self.load("save_slot_menu/starting_weapons.yaml")["records"][0]
        self.assertEqual(weapon["name"]["text"], "\uff21\uff21")
        self.assertTrue(weapon["name"]["bytes"].startswith("41 19 00 00"))
        self.assertEqual(self.load("game_state/new_game.yaml")["characters"][0]["name"]["text"], "\uff21\uff21")
        self.assertEqual(self.load("byte-map.yaml")["text_decoder"]["overlay"], "CLOAD")

    def test_missing_japanese_chart_writes_nothing(self):
        inputs = FakeOverlay("jp").write(self.root)
        (inputs.assets / "blob.databin.bin").unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            title.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_symbol_gives_a_rename_hint(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_save_layout_tim_04")
        with self.assertRaisesRegex(ValueError, "g_save_layout_tim_04.*TEXTURE_SYMBOL"):
            title.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_and_truncated_blob_write_nothing(self):
        inputs = self.overlay.write(self.root)
        path = inputs.assets / "title_data.databin.bin"
        path.write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            title.extract(inputs, self.output)
        path.unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            title.extract(inputs, self.output)
        self.assert_no_output()

    def test_symbols_out_of_order_write_nothing(self):
        sprite_uvs = title.SYMBOL_NAMES["sprite_uvs"]
        panel_uvs = title.SYMBOL_NAMES["panel_uvs"]
        self.overlay.symbols[sprite_uvs], self.overlay.symbols[panel_uvs] = (
            self.overlay.symbols[panel_uvs], self.overlay.symbols[sprite_uvs])
        with self.assertRaisesRegex(ValueError, "out of resource order"):
            title.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_bad_menu_offsets_write_nothing(self):
        with self.assertRaisesRegex(ValueError, "don't point at g_title_menu_tim_0"):
            title.extract(self.extract_with("menu_offsets", 8, 0x100), self.output)
        self.assert_no_output()

    def test_texture_pointers_must_match_the_tims(self):
        inputs = self.extract_with("texture_table", 0x18, ADDRESS)
        with self.assertRaisesRegex(ValueError, "doesn't point at the g_save_layout_tim_NN"):
            title.extract(inputs, self.output)
        self.assert_no_output()

    def test_tims_trailing_word_must_really_be_a_copy(self):
        self.overlay.data[self.overlay.offset("cursor_blink") - 1] ^= 1
        with self.assertRaisesRegex(ValueError, "does not duplicate"):
            title.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_starting_weapons_must_be_weapons(self):
        with self.assertRaisesRegex(ValueError, "starting weapon 0 is not a weapon record"):
            title.extract(self.extract_with("weapons", 0x14, 1 << 8), self.output)
        self.assert_no_output()

    def test_write_failure_removes_staging(self):
        with patch.object(title, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                title.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_existing_output_is_preserved(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            title.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
