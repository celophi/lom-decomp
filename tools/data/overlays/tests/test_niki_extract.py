"""Run the NIKI extractor end to end on a small made-up overlay.

The fake blob has one of everything the real one holds, in the same order,
and invented bytes throughout; no game files are read.
"""

import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
import zlib

import yaml

from tools.data.overlays import card_data, icon_set, niki
from tools.data.overlays.tests.test_overlay_tools import make_table

ADDRESS = 0x80147000
FULL_WIDTH_A = "Ａ".encode("shift_jis")
FULL_WIDTH_B = "Ｂ".encode("shift_jis")


def png_rows(path: Path) -> list[bytes]:
    """The unfiltered RGBA rows of a PNG written by tools.data.overlays.png."""
    raw = path.read_bytes()
    width, height = struct.unpack(">II", raw[16:24])
    length = struct.unpack(">I", raw[33:37])[0]
    assert raw[37:41] == b"IDAT"
    rows = zlib.decompress(raw[41 : 41 + length])
    stride = 1 + width * 4
    assert len(rows) == height * stride and all(rows[y * stride] == 0 for y in range(height))
    return [rows[y * stride + 1 : (y + 1) * stride] for y in range(height)]


def pad(data: bytearray, alignment: int = 4) -> None:
    data.extend(bytes(-len(data) % alignment))


class FakeOverlay:
    """Messages, titles, locations, one icon, steps, a chart, digits and variables."""

    def __init__(self, version: str = "us"):
        self.version = version
        names = {}
        data = bytearray()

        names["g_niki_text_table"] = 0
        names["g_niki_text_no_card"] = 2
        strings = [b"Load?", b"No card"] if version == "us" else [b"\x41\x19\x10", b"\x42"]
        data += make_table(strings)
        for title in ("ＭＡＮＡ", "ＢＡＤ"):
            pad(data)
            data += title.encode("shift_jis") + b"\x00"
        pad(data)
        names["g_niki_location_names"] = len(data)
        data += make_table([b"Home"] if version == "us" else [b"\x42\x41"])
        self.unknown_start = len(data)
        pad(data)
        data += bytes.fromhex("5a 5a 00 00")
        self.unknown_end = len(data)

        palette = struct.pack("<16H", 0, *range(0x7C00, 0x7C0F))
        pixels = bytes([0x21]) + bytes(icon_set.PIXEL_BYTES - 1)
        names["g_niki_icon_offsets"] = len(data) + 4
        data += struct.pack("<II", 1, 8) + palette + pixels
        data += data[-4:]

        names["g_niki_card_setup_sequence"] = len(data)
        data += bytes([3, 1, 2, 0])
        names["g_card_steps_idle"] = len(data)
        data += bytes([14, 0, 0, 0, 3])
        names["g_niki_write_save_sequence"] = len(data)
        data += bytes([30, 25, 26, 0])
        self.steps_end = len(data)

        names["g_glyph_single_byte_chart"] = len(data)
        row = FULL_WIDTH_A * card_data.CHART_COLUMNS + b"\n"
        data += row * (0x10 - (card_data.CHART_FIRST_CODE >> 4))
        # One two-byte page for lead 0x19, placed right after the one-byte rows.
        page = FULL_WIDTH_B * card_data.CHART_COLUMNS + b"\n"
        page_start = len(data)
        data += page * card_data.CHART_ROWS_PER_PAGE
        pad(data)
        names["g_glyph_chart_page_base"] = page_start - card_data.CHART_LEADS.start * card_data.CHART_PAGE_BYTES

        names["g_glyph_decimal_digits"] = len(data)
        data += "０１２３４５６７８９".encode("shift_jis") + b"\x00\x00"
        pad(data, 8)
        names["g_glyph_hex_digits"] = len(data)
        data += "０１２３４５６７８９ＡＢＣＤＥＦ".encode("shift_jis") + b"\x00\x00"
        pad(data, 8)
        names["g_niki_dialog_state"] = len(data)
        data += bytes(28)
        names["g_glyph_cursor_y"] = len(data)
        data += bytes(4)

        self.data = data
        self.symbols = {name: ADDRESS + offset for name, offset in names.items()}

    def write(self, root: Path, skip_symbol: str | None = None) -> niki.Inputs:
        config, assets = root / "config", root / "assets"
        (config / "symbols").mkdir(parents=True)
        (config / "overlays").mkdir()
        assets.mkdir()
        (assets / "niki_data.databin.bin").write_bytes(self.data)
        segment = {"start": 0x70F9, "vram": ADDRESS, "subsegments": [[0x70F9, "databin", "niki_data"]]}
        document = {"segments": [segment, [0x70F9 + len(self.data)]]}
        (config / niki.OVERLAY_CONFIG).write_text(yaml.safe_dump(document))
        lines = [f"{name} = 0x{address:08X};" for name, address in self.symbols.items() if name != skip_symbol]
        (config / niki.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return niki.Inputs(self.version, config, assets)


class ExtractTest(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        self.output = self.root / "out"
        self.overlay = FakeOverlay()

    def load(self, path: str) -> dict:
        return yaml.safe_load((self.output / path).read_text(encoding="utf-8"))

    def assert_no_output(self):
        self.assertFalse(self.output.exists())
        self.assertEqual(list(self.root.glob(".out-*")), [])

    def test_writes_text_titles_and_tables(self):
        niki.extract(self.overlay.write(self.root), self.output)
        messages = self.load("text/messages.yaml")["entries"]
        self.assertEqual([entry["text"] for entry in messages], ["Load?", "No card"])
        self.assertEqual(messages[0]["symbol"], "g_niki_text_table")
        self.assertEqual(messages[1]["symbol"], "g_niki_text_no_card")
        titles = self.load("text/card_titles.yaml")["titles"]
        self.assertEqual([title["text"] for title in titles], ["ＭＡＮＡ", "ＢＡＤ"])
        self.assertEqual(self.load("text/locations.yaml")["entries"][0]["text"], "Home")
        chart = self.load("tables/text_conversion.yaml")
        self.assertEqual(chart["one_byte"]["0x4_"], "Ａ" * 16)
        self.assertEqual(chart["two_byte"]["0x19"]["0x1_"], "Ｂ" * 16)
        digits = self.load("tables/digit_glyphs.yaml")
        self.assertEqual(digits["hexadecimal"]["glyphs"][-1], "Ｆ")

    def test_card_steps_follow_each_entry_and_list_unreached_bytes(self):
        niki.extract(self.overlay.write(self.root), self.output)
        steps = self.load("tables/card_steps.yaml")
        sequences = {sequence["symbol"]: sequence for sequence in steps["sequences"]}
        self.assertEqual(
            sequences["g_niki_card_setup_sequence"]["steps"],
            ["CARD_MENU_STEP_CLEAR_SOFTWARE_EVENTS", "CARD_MENU_STEP_CARD_INFO",
             "CARD_MENU_STEP_POLL_CARD_INFO", "CARD_MENU_STEP_DONE"],
        )
        self.assertEqual(sequences["g_card_steps_idle"]["steps"], ["CARD_MENU_STEP_WAIT", "CARD_MENU_STEP_DONE"])
        self.assertEqual(sequences["g_niki_write_save_sequence"]["bytes"], "1e 19 1a 00")
        address = self.overlay.symbols["g_niki_write_save_sequence"] - 1
        self.assertEqual(steps["unreached"], [{"address": f"0x{address:08X}", "step": "CARD_MENU_STEP_CLEAR_SOFTWARE_EVENTS"}])

    def test_icon_png_uses_the_stored_palette(self):
        niki.extract(self.overlay.write(self.root), self.output)
        icons = self.load("icons/icons.yaml")["icons"]
        self.assertEqual([icon["file"] for icon in icons], ["icon_00.png"])
        self.assertEqual(icons[0]["palette"][1], "0x7C00")
        rows = png_rows(self.output / "icons/icon_00.png")
        self.assertEqual(len(rows), icon_set.ICON_SIZE)
        self.assertEqual(rows[0][0:4], bytes((0, 0, 255, 255)))  # low nibble 1: 0x7C00, blue
        self.assertEqual(rows[0][4:8], bytes((8, 0, 255, 255)))  # high nibble 2: 0x7C01
        self.assertEqual(rows[0][8:12][3], 0)  # color 0 is transparent
        self.assertEqual(rows[1], bytes(icon_set.ICON_SIZE * 4))

    def test_japanese_text_decodes_through_the_chart(self):
        niki.extract(FakeOverlay("jp").write(self.root), self.output)
        message = self.load("text/messages.yaml")["entries"][0]
        self.assertEqual(message["text"], "ＡＢ")
        self.assertEqual(message["bytes"], "41 19 10")
        self.assertEqual(self.load("text/locations.yaml")["entries"][0]["text"], "ＡＡ")

    def test_byte_map_covers_every_byte(self):
        niki.extract(self.overlay.write(self.root), self.output)
        ranges = self.load("byte-map.yaml")["ranges"]
        cursor = 0
        for part in ranges:
            self.assertEqual(int(part["offset"], 16), cursor, part["name"])
            cursor += int(part["size"], 16)
        self.assertEqual(cursor, len(self.overlay.data))
        self.assertEqual(ranges[-1]["name"], "variables")
        self.assertNotIn("file", ranges[-1])
        address = ADDRESS + self.overlay.unknown_start
        unknown = self.overlay.data[self.overlay.unknown_start : self.overlay.unknown_end]
        self.assertEqual((self.output / f"unknown/{address:08X}.bin").read_bytes(), unknown)
        self.assertTrue(unknown.endswith(bytes.fromhex("5a 5a 00 00")))
        self.assertIn("icon set trailing word", [part["name"] for part in ranges])

    def test_nonzero_variables_are_saved(self):
        self.overlay.data[-1] = 0x5A
        niki.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "unknown/variables.bin").read_bytes()[-1:], b"Z")

    def test_missing_symbol_reports_what_to_update(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_niki_location_names")
        with self.assertRaisesRegex(ValueError, "g_niki_location_names.*SYMBOL_NAMES"):
            niki.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_blob_reports_make_splat(self):
        inputs = self.overlay.write(self.root)
        (inputs.assets / "niki_data.databin.bin").unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            niki.extract(inputs, self.output)
        self.assert_no_output()

    def test_truncated_blob_writes_nothing(self):
        inputs = self.overlay.write(self.root)
        (inputs.assets / "niki_data.databin.bin").write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            niki.extract(inputs, self.output)
        self.assert_no_output()

    def test_out_of_order_symbols_are_rejected(self):
        self.overlay.symbols["g_niki_location_names"] = ADDRESS
        with self.assertRaisesRegex(ValueError, "g_niki_location_names is out of resource order"):
            niki.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_bad_text_offset_writes_nothing(self):
        self.overlay.data[2:4] = struct.pack("<H", 0xFFF0)
        with self.assertRaisesRegex(ValueError, "invalid offset"):
            niki.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_bad_icon_offset_writes_nothing(self):
        start = self.overlay.symbols["g_niki_icon_offsets"] - ADDRESS
        self.overlay.data[start : start + 4] = struct.pack("<I", 0x100000)
        with self.assertRaisesRegex(ValueError, "icon 0 has an invalid offset"):
            niki.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_sequence_cannot_run_into_the_chart(self):
        end = self.overlay.steps_end
        self.overlay.data[end - 1] = 26
        with self.assertRaisesRegex(ValueError, "g_niki_write_save_sequence has no CARD_MENU_STEP_DONE"):
            niki.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_write_failure_removes_staging(self):
        with patch.object(niki, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                niki.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_existing_output_is_refused(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            niki.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
