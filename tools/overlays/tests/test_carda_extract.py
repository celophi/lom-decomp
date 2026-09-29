"""Exercise CARDA's exporter without disc files.

The small overlay has both icon formats, each text table, and a card sequence
with an entry point in the middle. That last case matters: a symbol isn't
necessarily the end of the sequence before it.
"""

import struct
import tempfile
import unittest
import zlib
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.overlays import card_data, carda, icon_set
from tools.overlays.tests.test_overlay_tools import make_table

ADDRESS = 0x80140000
FULL_WIDTH_A = bytes.fromhex("82 60")


def pad(data: bytearray) -> None:
    data.extend(bytes(-len(data) % 4))


class FakeOverlay:
    """Build CARDA's data blob, fixed strings and the config that points to them."""

    def __init__(self, version="us"):
        self.version = version
        small = bytearray()
        self.symbols = {}
        for key, raw in (
            ("title_dash", bytes.fromhex("81 7c")),
            ("title_colon", bytes.fromhex("81 46")),
            ("save_card_path", b"bu00:"),
            ("overflow_text", bytes.fromhex("82 6c 82 60 82 77")),
            ("card_path", b"bu00:"),
            ("directory_pattern", b"bu00:*"),
        ):
            self.symbols[carda.SYMBOL_NAMES[key]] = ADDRESS + len(small)
            small += raw + b"\x00"
            pad(small)
        self.small = bytes(small)
        self.address = ADDRESS + len(small)
        data = bytearray()

        def mark(key):
            self.symbols[carda.SYMBOL_NAMES[key]] = self.address + len(data)

        mark("messages")
        self.symbols["g_carda_text_no_card"] = self.address + 2
        strings = [b"Save?", b"No card"] if version == "us" else [b"\x41\x19\x00", b"\x41"]
        data += make_table(strings)
        pad(data)
        mark("items")
        data += make_table([b"Item"] if version == "us" else [b"\x41"])
        pad(data)
        for key in ("save_title", "bad_title"):
            mark(key)
            data += FULL_WIDTH_A + b"\x00"
            pad(data)

        palette = struct.pack("<16H", 0, 0x001F, 0x03E0, 0x7C00, *([0x7FFF] * 12))
        self.save_start = len(data)
        data += struct.pack("<I", 1)
        mark("save_icon_offsets")
        data += struct.pack("<I", 8) + palette
        data += b"\x21" + bytes(carda.SAVE_ICON_FRAME_BYTES - 1)
        data += b"\x03" + bytes([0x33]) * (carda.SAVE_ICON_FRAME_BYTES - 1)
        data += data[-4:]
        mark("locations")
        data += make_table([b"Home"] if version == "us" else [b"\x41"])
        pad(data)
        data += struct.pack("<I", 1)
        mark("icon_offsets")
        data += struct.pack("<I", 8) + palette + bytes([0x21]) * icon_set.PIXEL_BYTES
        data += data[-4:]

        mark("card_steps")
        data += bytes([1, 2, 0, 0])
        self.symbols["g_card_steps_idle"] = self.address + len(data)
        data += bytes([14, 0, 0, 0])
        self.symbols["g_carda_steps_write_save"] = self.address + len(data)
        self.symbols["g_carda_steps_write_save_keep_handles"] = self.address + len(data) + 1
        data += bytes([3, 10, 30, 11, 12, 13, 0, 0])
        self.steps_end = len(data)
        mark("chart")
        row = FULL_WIDTH_A * card_data.CHART_COLUMNS + b"\n"
        data += row * (16 - (card_data.CHART_FIRST_CODE >> 4))
        self.symbols[carda.SYMBOL_NAMES["chart_pages"]] = (
            self.address + len(data) - 0x19 * card_data.CHART_PAGE_BYTES
        )
        data += row * card_data.CHART_ROWS_PER_PAGE
        pad(data)
        mark("decimal_glyphs")
        data += b"".join(bytes([0x82, 0x4F + i]) for i in range(10)) + bytes(4)
        mark("hex_glyphs")
        data += b"".join(bytes([0x82, 0x4F + i]) for i in range(10))
        data += b"".join(bytes([0x82, 0x60 + i]) for i in range(6)) + bytes(4)
        self.symbols["g_carda_scroll_target_y"] = self.address + len(data)
        data += bytes(32)
        self.data = data

    def write(self, root: Path, skip_symbol=None) -> carda.Inputs:
        config, assets = root / "config", root / "assets"
        (config / "symbols").mkdir(parents=True)
        (config / "overlays").mkdir()
        assets.mkdir()
        (assets / "small.rodatabin.bin").write_bytes(self.small)
        (assets / "blob.databin.bin").write_bytes(self.data)
        segment = {
            "name": "carda",
            "type": "decompress_overlay",
            "start": 1,
            "vram": ADDRESS,
            "subsegments": [[1, "rodatabin", "small"], [1 + len(self.small), "databin", "blob"]],
        }
        end = 1 + len(self.small) + len(self.data)
        (config / carda.OVERLAY_CONFIG).write_text(yaml.safe_dump({"segments": [segment, [end]]}))
        lines = [
            f"{name} = 0x{address:08X};"
            for name, address in self.symbols.items() if name != skip_symbol
        ]
        (config / carda.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return carda.Inputs(self.version, config, assets)


class ExtractTest(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        self.overlay = FakeOverlay()
        self.output = self.root / "out"

    def load(self, path):
        return yaml.safe_load((self.output / path).read_text(encoding="utf-8"))

    def assert_no_output(self):
        self.assertFalse(self.output.exists())
        self.assertEqual(list(self.root.glob(".out-*")), [])

    def test_writes_all_tables_and_both_icon_formats(self):
        carda.extract(self.overlay.write(self.root), self.output)
        messages = self.load("text/messages.yaml")["entries"]
        self.assertEqual([entry["text"] for entry in messages], ["Save?", "No card"])
        self.assertEqual(messages[1]["symbol"], "g_carda_text_no_card")
        self.assertEqual(self.load("text/items.yaml")["entries"][0]["text"], "Item")
        self.assertEqual(self.load("text/locations.yaml")["entries"][0]["text"], "Home")
        self.assertEqual(len(self.load("text/card_titles.yaml")["titles"]), 2)
        self.assertEqual(len(self.load("text/fixed_strings.yaml")["strings"]), 6)
        self.assertEqual(len(self.load("tables/digit_glyphs.yaml")["hexadecimal"]["glyphs"]), 16)
        self.assertEqual(self.load("tables/text_conversion.yaml")["one_byte"]["0x4_"], "\uff21" * 16)
        self.assertEqual(len(self.load("icons/icons.yaml")["icons"]), 1)
        self.assertTrue((self.output / "icons/icon_00.png").exists())
        icon = self.load("save_icons/icons.yaml")["icons"][0]
        self.assertEqual(icon["frames"], ["icon_00_0.png", "icon_00_1.png"])
        # Check frame order, nibble order and transparency in the actual exported PNGs.
        expected = (bytes([255, 0, 0, 255, 0, 255, 0, 255]), bytes([0, 0, 255, 255, 0, 0, 0, 0]))
        for name, colors in zip(icon["frames"], expected):
            data = (self.output / "save_icons" / name).read_bytes()
            self.assertEqual(struct.unpack(">II", data[16:24]), (16, 16))
            start = data.index(b"IDAT") + 4
            length = struct.unpack(">I", data[start - 8 : start - 4])[0]
            self.assertEqual(zlib.decompress(data[start : start + length])[1:9], colors)

    def test_japanese_text_uses_the_chart_even_when_a_second_byte_is_zero(self):
        carda.extract(FakeOverlay("jp").write(self.root), self.output)
        entries = self.load("text/messages.yaml")["entries"]
        self.assertEqual(entries[0]["text"], "\uff21\uff21")
        self.assertEqual(entries[0]["bytes"], "41 19 00")

    def test_an_entry_inside_a_sequence_does_not_cut_it_short(self):
        carda.extract(self.overlay.write(self.root), self.output)
        sequences = {s["symbol"]: s for s in self.load("tables/card_steps.yaml")["sequences"]}
        outer = sequences["g_carda_steps_write_save"]
        inner = sequences["g_carda_steps_write_save_keep_handles"]
        self.assertEqual(outer["bytes"], "03 0a 1e 0b 0c 0d 00")
        self.assertEqual(inner["steps"], outer["steps"][1:])
        self.assertEqual(inner["steps"][-1], "CARDA_STEP_DONE")

    def test_byte_map_covers_the_whole_blob_once(self):
        carda.extract(self.overlay.write(self.root), self.output)
        cursor = 0
        ranges = self.load("byte-map.yaml")["ranges"]
        for part in ranges:
            self.assertEqual(int(part["offset"], 16), cursor, part["name"])
            cursor += int(part["size"], 16)
        self.assertEqual(cursor, len(self.overlay.data))
        self.assertIn("save icon set trailing word", [part["name"] for part in ranges])
        self.assertEqual(ranges[-1]["name"], "variables")
        self.assertFalse((self.output / "unknown").exists())

    def test_nonzero_variables_are_preserved(self):
        self.overlay.data[-1] = 0x5A
        carda.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "unknown/variables.bin").read_bytes(), bytes(31) + b"Z")

    def test_missing_symbol_reports_the_name_and_writes_nothing(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_carda_location_names")
        with self.assertRaisesRegex(ValueError, "g_carda_location_names.*SYMBOL_NAMES"):
            carda.extract(inputs, self.output)
        self.assert_no_output()

    def test_truncated_blob_writes_nothing(self):
        inputs = self.overlay.write(self.root)
        (inputs.assets / "blob.databin.bin").write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            carda.extract(inputs, self.output)
        self.assert_no_output()

    def test_save_icon_cannot_read_the_location_table_as_pixels(self):
        struct.pack_into("<I", self.overlay.data, self.overlay.save_start + 4, 16)
        with self.assertRaisesRegex(ValueError, "save icon 0 has an invalid offset"):
            carda.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_sequence_without_done_is_rejected(self):
        self.overlay.data[self.overlay.steps_end - 8 : self.overlay.steps_end] = bytes([3]) * 8
        with self.assertRaisesRegex(ValueError, "g_carda_steps_write_save has no CARDA_STEP_DONE"):
            carda.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_write_failure_removes_the_temporary_folder(self):
        with patch.object(carda, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                carda.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_existing_output_is_left_alone(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            carda.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
