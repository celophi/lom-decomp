"""Tests for the ZUKAN extractor, run against a small made-up overlay instead of disc files."""

import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.data.formats.psx_tim import TIM_FLAG_HAS_CLUT, TimBlock, TimImage
from tools.data.overlays import zukan
from tools.data.overlays.tests.test_cload_extract import FakeOverlay as FakeCload
from tools.data.overlays.tests.test_field_extract import png_pixels
from tools.data.overlays.tests.test_overlay_tools import make_table

OVERLAY_ADDRESS = 0x80140000
TABLES_ADDRESS = 0x80140020
DATA_ADDRESS = 0x80144000

ENTRY_COUNT = 1018
CATEGORY_BOUNDARIES = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13]
HISTORY_GROUP_OFFSETS = [0, 1, 2, 3, 4, 5, 6]
DISPLAY_ORDER = [2, 1, zukan.DISPLAY_ORDER_END]
# The two bytes between the last table and the code.
TABLE_PADDING = 2

# PS1 colors are 15-bit BGR. The PNGs come out as RGBA.
TRANSPARENT, RED, GREEN, BLUE, WHITE = 0x0000, 0x001F, 0x03E0, 0x7C00, 0x7FFF
RGBA = {TRANSPARENT: (0, 0, 0, 0), RED: (255, 0, 0, 255), GREEN: (0, 255, 0, 255), BLUE: (0, 0, 255, 255)}

PAGE_PALETTES = [
    [TRANSPARENT, RED, GREEN, BLUE] + [WHITE] * 12,
    [TRANSPARENT, BLUE] + [GREEN] * 14,
]
# The screen never uses this palette: the page texture's CLUT overwrites it.
BORDER_PALETTES = [[TRANSPARENT] + [RED] * 15]

# 4-bit pixels, two to a byte with the low nibble first.
PAGE_PIXELS = bytes.fromhex("10 32 21 03 32 10 03 21")  # first row: 0, 1, 2, 3, 1, 2, 3, 0
BORDER_PIXELS = bytes.fromhex("12 21 21 12")  # first row: 2, 1, 1, 2

# JP text goes through CLOAD's chart. 0x41 is a one-byte code, and 0x19 starts a
# two-byte code whose second byte is 0.
FULL_WIDTH_A = "Ａ"


def four_bit_tim(palettes: list[list[int]], pixels: bytes, width_words: int) -> TimImage:
    """A 4-bit TIM with its CLUT at (0, 480). Each 16-bit word of a row holds four pixels."""
    colors = [color for palette in palettes for color in palette]
    clut = TimBlock(0, 480, len(colors), 1, struct.pack(f"<{len(colors)}H", *colors))
    height = len(pixels) // (width_words * 2)
    return TimImage(TIM_FLAG_HAS_CLUT, clut, TimBlock(0, 0, width_words, height, pixels))


def sprite_record(texture, u, v, palette, width, height, x, y) -> bytes:
    """One UI sprite: two packed words, then the screen position."""
    first = texture | u << 8
    second = v | palette << 8 | width << 14 | height << 23
    return struct.pack("<IIHH", first, second, x, y)


def archive(sections: list[bytes]) -> bytes:
    """The section count, one offset per section, then the sections themselves."""
    offsets = []
    position = zukan.ARCHIVE_HEADER.size
    for section in sections:
        offsets.append(position)
        position += len(section)
    return zukan.ARCHIVE_HEADER.pack(len(sections), *offsets) + b"".join(sections)


def first_row_colors(path: Path) -> list[tuple[int, int, int, int]]:
    """The RGBA colors of a PNG's first row."""
    width, _, rows = png_pixels(path)
    row = rows[1 : 1 + width * 4]  # skip the filter byte
    return [tuple(row[i : i + 4]) for i in range(0, len(row), 4)]


class FakeOverlay:
    """A ZUKAN overlay with every resource the extractor reads, kept small."""

    def __init__(self, version="us"):
        self.version = version
        self.symbols = {}
        self.page = four_bit_tim(PAGE_PALETTES, PAGE_PIXELS, width_words=2)
        self.border = four_bit_tim(BORDER_PALETTES, BORDER_PIXELS, width_words=1)
        self.data = self.build_data()
        self.tables = self.build_tables()

    def build_data(self) -> bytearray:
        """The archive, the sprites and the screen's variables, in that order."""
        if self.version == "us":
            names = [b"----", b"Home", b"Domina"] + [b"Entry"] * 10
            categories = [b""] + [f"Category {i}".encode() for i in range(1, 12)]
        else:
            names = [b"\x41\x19\x00"] + [b"\x41"] * 12
            categories = [b"\x41"] * 12
        names = make_table(names)
        names += bytes(-len(names) % 4)

        data = bytearray()

        def mark(key):
            self.symbols[zukan.SYMBOL_NAMES[key]] = DATA_ADDRESS + len(data)

        mark("archive")
        self.archive = archive([self.page.to_bytes(), self.border.to_bytes(), names, make_table(categories)])
        data += self.archive
        # Padding after the archive, plus four spare bytes the extractor has to account for.
        self.tail_start = len(data)
        data += bytes(-len(data) % 4 + 4)

        mark("sprites")
        for index in range(zukan.SPRITE_COUNT):
            data += sprite_record(texture=index % 2, u=0, v=0, palette=index % 2, width=4, height=2, x=index, y=200)
        data += bytes(4)

        mark("variables")
        data += bytes(64)
        return data

    def build_tables(self) -> bytearray:
        """The four entry tables, back to back, then the padding before the code."""
        tables = {
            "category_ranges": struct.pack(f"<{len(CATEGORY_BOUNDARIES)}i", *CATEGORY_BOUNDARIES),
            "group_ranges": struct.pack(f"<{len(HISTORY_GROUP_OFFSETS)}i", *HISTORY_GROUP_OFFSETS),
            # Every fifth entry has no page.
            "entry_values": struct.pack(f"<{ENTRY_COUNT}h", *[0 if i % 5 == 0 else i for i in range(ENTRY_COUNT)]),
            "display_order": struct.pack(f"<{len(DISPLAY_ORDER)}H", *DISPLAY_ORDER),
        }
        data = bytearray()
        for key, table in tables.items():
            self.symbols[zukan.SYMBOL_NAMES[key]] = TABLES_ADDRESS + len(data)
            data += table
        return data + bytes(TABLE_PADDING)

    def write(self, root: Path, skip_symbol=None) -> zukan.Inputs:
        """Lay the overlay out the way make splat leaves it, and return the extractor's inputs."""
        config, assets = root / "config", root / "assets"
        if self.version == "jp":
            FakeCload("jp").write(root)
        else:
            (config / "symbols").mkdir(parents=True)
            (config / "overlays").mkdir()
            assets.mkdir()

        (assets / "zukan_data.databin.bin").write_bytes(self.data)
        self.write_image(root / "build/ZUKAN.BIN")
        self.write_splat_config(config / zukan.OVERLAY_CONFIG)

        lines = [f"{name} = 0x{address:08X};" for name, address in self.symbols.items() if name != skip_symbol]
        (config / zukan.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return zukan.Inputs(self.version, config, assets)

    def write_image(self, path: Path) -> None:
        """The decompressed overlay. The tables are the only part the extractor reads from it."""
        image = bytearray(rom_offset(TABLES_ADDRESS)) + self.tables
        path.parent.mkdir(exist_ok=True)
        path.write_bytes(image)

    def write_splat_config(self, path: Path) -> None:
        code_start = rom_offset(TABLES_ADDRESS) + len(self.tables)
        segment = {
            "start": 1,
            "vram": OVERLAY_ADDRESS,
            "subsegments": [
                [rom_offset(TABLES_ADDRESS), ".rodata", "zukan_category"],
                [code_start, "c", "zukan"],
                [rom_offset(DATA_ADDRESS), "databin", "zukan_data"],
            ],
        }
        end = rom_offset(DATA_ADDRESS) + len(self.data)
        options = {"base_path": "../..", "target_path": "build/ZUKAN.BIN"}
        path.write_text(yaml.safe_dump({"options": options, "segments": [segment, [end]]}))


def rom_offset(address: int) -> int:
    """Where an address sits in the decompressed overlay, which keeps its format byte at offset 0."""
    return address - OVERLAY_ADDRESS + 1


class ExtractTest(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        self.output = self.root / "out"
        self.overlay = FakeOverlay()

    def extract(self, overlay=None):
        zukan.extract((overlay or self.overlay).write(self.root), self.output)

    def load(self, name):
        return yaml.safe_load((self.output / name).read_text(encoding="utf-8"))

    def assert_fails_without_output(self, error, message, inputs=None):
        with self.assertRaisesRegex(error, message):
            zukan.extract(inputs or self.overlay.write(self.root), self.output)
        self.assertFalse(self.output.exists())
        self.assertEqual(list(self.root.glob(".out-*")), [])

    def assert_covers_every_byte(self, byte_map, size):
        position = 0
        for part in byte_map["ranges"]:
            self.assertEqual(int(part["offset"], 16), position)
            position += int(part["size"], 16)
            if "file" in part:
                self.assertTrue((self.output / part["file"]).is_file())
        self.assertEqual(position, size)

    def test_byte_maps_cover_the_data_and_the_tables(self):
        self.extract()
        mapping = self.load("byte-map.yaml")
        self.assert_covers_every_byte(mapping, len(self.overlay.data))
        self.assert_covers_every_byte(mapping["rodata"][0], len(self.overlay.tables))

        # The screen's variables are all zero, so they get no file of their own.
        self.assertEqual(mapping["ranges"][-1]["name"], "screen state")
        self.assertNotIn("file", mapping["ranges"][-1])
        self.assertFalse((self.output / "unknown").exists())

    def test_textures_keep_the_original_tims(self):
        self.extract()
        self.assertEqual((self.output / "images/page/image.tim").read_bytes(), self.overlay.page.to_bytes())
        self.assertEqual((self.output / "images/border/image.tim").read_bytes(), self.overlay.border.to_bytes())

        sections = self.load("archive/archive.yaml")["sections"]
        self.assertEqual([section["name"] for section in sections], list(zukan.ARCHIVE_SECTIONS))

        border = self.load("images/border/image.yaml")
        self.assertEqual(border["upload"], {"pixels": [832, 256], "clut": [0, 498]})
        self.assertEqual([preview["file"] for preview in border["previews"]], ["palette_00.png", "palette_01.png"])

    def test_sprites_are_decoded(self):
        self.extract()
        sprites = self.load("images/sprites/sprites.yaml")["entries"]
        self.assertEqual(len(sprites), zukan.SPRITE_COUNT)
        decoded = {key: sprites[1][key] for key in ("texture", "u", "v", "width", "height", "palette", "x", "y")}
        self.assertEqual(decoded, {"texture": "border", "u": 0, "v": 0, "width": 4, "height": 2, "palette": 1,
                                   "x": 1, "y": 200})

    def test_sprites_use_the_page_palettes_on_both_textures(self):
        self.extract()
        # Sprite 0 is on the page texture with page palette 0.
        page_sprite = first_row_colors(self.output / "images/sprites/00.png")
        self.assertEqual(page_sprite, [RGBA[TRANSPARENT], RGBA[RED], RGBA[GREEN], RGBA[BLUE]])
        # Sprite 1 is on the border texture, but still comes out in page palette 1, not the border's red.
        border_sprite = first_row_colors(self.output / "images/sprites/01.png")
        self.assertEqual(border_sprite, [RGBA[GREEN], RGBA[BLUE], RGBA[BLUE], RGBA[GREEN]])

    def test_entry_and_category_names(self):
        self.extract()
        names = self.load("text/entry_names.yaml")["entries"]
        self.assertEqual([entry["text"] for entry in names[:3]], ["----", "Home", "Domina"])

        categories = self.load("tables/categories.yaml")["entries"]
        self.assertEqual(len(categories), 12)
        self.assertEqual(categories[1], {"index": 1, "name": "Category 1", "first_entry": 1, "stored_end": 2,
                                         "list_end": 2, "first_name": "Home"})

    def test_category_overrides(self):
        self.extract()
        categories = self.load("tables/categories.yaml")["entries"]
        self.assertEqual(categories[7]["list_end"], 454)
        self.assertEqual(categories[8]["list_end"], 576)
        self.assertEqual(categories[11]["extra_entries"], [454, 480])

    def test_entries_and_their_pages(self):
        self.extract()
        entries = self.load("tables/entries.yaml")["entries"]
        self.assertEqual(len(entries), ENTRY_COUNT)
        self.assertEqual(entries[2], {"index": 2, "name": "Domina", "category": 2, "value": 2, "cd_resource": "0xBFE"})
        self.assertNotIn("cd_resource", entries[5])
        self.assertIsNone(entries[20]["name"])

    def test_display_order_and_history_groups(self):
        self.extract()
        order = self.load("tables/display_order.yaml")
        self.assertEqual([entry["entry"] for entry in order["entries"]], [2, 1])

        groups = self.load("tables/history_groups.yaml")["entries"]
        self.assertEqual(groups[5], {"index": 5, "first_entry": 485, "end_entry": 486, "first_name": None,
                                     "unlock_bit": "0x127"})

    def test_japanese_text_is_decoded_with_cload(self):
        self.extract(FakeOverlay("jp"))
        entry = self.load("text/entry_names.yaml")["entries"][0]
        self.assertEqual(entry["text"], FULL_WIDTH_A * 2)
        self.assertEqual(entry["bytes"], "41 19 00")
        self.assertEqual(self.load("tables/categories.yaml")["entries"][3]["name"], FULL_WIDTH_A)
        self.assertEqual(self.load("byte-map.yaml")["text_decoder"]["overlay"], "CLOAD")

    def test_unknown_bytes_after_the_archive_are_saved(self):
        self.overlay.data[self.overlay.tail_start + 1] = 0x73
        self.extract()
        address = DATA_ADDRESS + self.overlay.tail_start
        self.assertEqual((self.output / f"unknown/{address:08X}.bin").read_bytes()[:2], b"\x00\x73")

    def test_missing_symbol_says_what_to_update(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_zukan_display_order")
        self.assert_fails_without_output(ValueError, "g_zukan_display_order.*SYMBOL_NAMES", inputs)

    def test_truncated_data_blob(self):
        inputs = self.overlay.write(self.root)
        (inputs.assets / "zukan_data.databin.bin").write_bytes(self.overlay.data[:-1])
        self.assert_fails_without_output(ValueError, "size does not match", inputs)

    def test_missing_overlay_image(self):
        inputs = self.overlay.write(self.root)
        (self.root / "build/ZUKAN.BIN").unlink()
        self.assert_fails_without_output(ValueError, "run make splat first", inputs)

    def test_missing_japanese_chart(self):
        inputs = FakeOverlay("jp").write(self.root)
        (inputs.assets / "blob.databin.bin").unlink()
        self.assert_fails_without_output(ValueError, "run make splat first", inputs)

    def test_archive_offset_past_the_end(self):
        last_offset = zukan.ARCHIVE_HEADER.size - 4
        struct.pack_into("<I", self.overlay.data, last_offset, len(self.overlay.archive) + 0x100)
        self.assert_fails_without_output(ValueError, "section offsets")

    def test_sprite_outside_its_texture(self):
        start = self.overlay.symbols[zukan.SYMBOL_NAMES["sprites"]] - DATA_ADDRESS
        # u = 6 with a width of 4 runs past the page texture's 8 pixels.
        record = sprite_record(texture=0, u=6, v=0, palette=0, width=4, height=2, x=0, y=0)
        self.overlay.data[start : start + len(record)] = record
        self.assert_fails_without_output(ValueError, "UI sprite 0 lies outside")

    def test_display_order_without_terminator(self):
        start = self.overlay.symbols[zukan.SYMBOL_NAMES["display_order"]] - TABLES_ADDRESS
        terminator = start + DISPLAY_ORDER.index(zukan.DISPLAY_ORDER_END) * 2
        struct.pack_into("<H", self.overlay.tables, terminator, 0)
        self.assert_fails_without_output(ValueError, "terminator")

    def test_failed_write_leaves_nothing_behind(self):
        with patch.object(zukan, "write_part", side_effect=OSError("disk full")):
            self.assert_fails_without_output(OSError, "disk full")

    def test_existing_output_is_left_alone(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            self.extract()
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
