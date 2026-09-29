"""Exercise CLOAD's exporter with a small made-up blob, without disc files."""

import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.overlays import card_data, cload
from tools.overlays.tests.test_overlay_tools import make_table

ADDRESS = 0x80140000
FULL_WIDTH_A = bytes.fromhex("82 60")


class FakeOverlay:
    """Text, step entries, two chart pages, digits and a runtime buffer."""

    def __init__(self, version="us"):
        self.version = version
        self.symbols = {}
        data = bytearray()

        def mark(key):
            self.symbols[cload.SYMBOL_NAMES[key]] = ADDRESS + len(data)

        def pad():
            data.extend(bytes(-len(data) % 4))

        mark("messages")
        self.symbols["g_cload_text_no_memory_card"] = ADDRESS + 2
        strings = [b"Load?", b"No card"] if version == "us" else [b"\x41\x19\x00", b"\x41"]
        data += make_table(strings)
        self.unknown_start = len(data)
        pad()
        data += bytes.fromhex("12 34 56 78")
        mark("locations")
        data += make_table([b"Home"] if version == "us" else [b"\x41"])
        pad()
        mark("card_steps")
        data += bytes([3, 1, 2, 0])
        self.symbols["g_cload_steps_card_reset"] = ADDRESS + len(data) - 3
        self.symbols["g_cload_steps_idle"] = ADDRESS + len(data)
        data += bytes([14, 0, 0, 0])
        self.steps_end = len(data)
        mark("chart")
        row = FULL_WIDTH_A * card_data.CHART_COLUMNS + b"\n"
        data += row * (16 - (card_data.CHART_FIRST_CODE >> 4))
        pad()
        mark("double_byte_chart")
        data += row * card_data.CHART_ROWS_PER_PAGE
        pad()
        mark("decimal_glyphs")
        data += b"".join(bytes([0x82, 0x4F + i]) for i in range(10)) + bytes(4)
        mark("hex_glyphs")
        data += b"".join(bytes([0x82, 0x4F + i]) for i in range(10))
        data += b"".join(bytes([0x82, 0x60 + i]) for i in range(6)) + bytes(4)
        mark("variables")
        data += bytes(32)
        self.data = data

    def write(self, root: Path, skip_symbol=None) -> cload.Inputs:
        config, assets = root / "config", root / "assets"
        (config / "symbols").mkdir(parents=True)
        (config / "overlays").mkdir()
        assets.mkdir()
        (assets / "blob.databin.bin").write_bytes(self.data)
        segment = {
            "start": 1,
            "vram": ADDRESS,
            "subsegments": [[1, "databin", "blob"]],
        }
        document = {"segments": [segment, [1 + len(self.data)]]}
        (config / cload.OVERLAY_CONFIG).write_text(yaml.safe_dump(document))
        lines = [
            f"{name} = 0x{address:08X};"
            for name, address in self.symbols.items() if name != skip_symbol
        ]
        (config / cload.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return cload.Inputs(self.version, config, assets)


class ExtractTest(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        self.output = self.root / "out"
        self.overlay = FakeOverlay()

    def load(self, path):
        return yaml.safe_load((self.output / path).read_text(encoding="utf-8"))

    def assert_no_output(self):
        self.assertFalse(self.output.exists())
        self.assertEqual(list(self.root.glob(".out-*")), [])

    def test_writes_text_steps_charts_and_digits(self):
        cload.extract(self.overlay.write(self.root), self.output)
        messages = self.load("text/messages.yaml")["entries"]
        self.assertEqual([entry["text"] for entry in messages], ["Load?", "No card"])
        self.assertEqual(messages[1]["symbol"], "g_cload_text_no_memory_card")
        self.assertEqual(self.load("text/locations.yaml")["entries"][0]["text"], "Home")
        chart = self.load("tables/text_conversion.yaml")
        self.assertEqual(chart["one_byte"]["0x4_"], "\uff21" * 16)
        self.assertEqual(chart["two_byte"]["0x19"]["0x0_"], "\uff21" * 16)
        digits = self.load("tables/digit_glyphs.yaml")
        self.assertEqual(len(digits["decimal"]["glyphs"]), 10)
        self.assertEqual(len(digits["hexadecimal"]["glyphs"]), 16)
        sequences = self.load("tables/card_steps.yaml")["sequences"]
        self.assertEqual(sequences[0]["bytes"], "03 01 02 00")
        self.assertEqual(sequences[1]["steps"], sequences[0]["steps"][1:])
        self.assertEqual(sequences[2]["steps"], ["CLOAD_STEP_IDLE", "CLOAD_STEP_DONE"])
        self.assertFalse((self.output / "icons").exists())

    def test_japanese_text_keeps_a_zero_second_byte(self):
        cload.extract(FakeOverlay("jp").write(self.root), self.output)
        message = self.load("text/messages.yaml")["entries"][0]
        self.assertEqual(message["text"], "\uff21\uff21")
        self.assertEqual(message["bytes"], "41 19 00")

    def test_byte_map_covers_every_byte_and_preserves_unknown_data(self):
        cload.extract(self.overlay.write(self.root), self.output)
        cursor = 0
        ranges = self.load("byte-map.yaml")["ranges"]
        for part in ranges:
            self.assertEqual(int(part["offset"], 16), cursor, part["name"])
            cursor += int(part["size"], 16)
        self.assertEqual(cursor, len(self.overlay.data))
        self.assertEqual(ranges[-1]["name"], "variables")
        self.assertNotIn("file", ranges[-1])
        address = ADDRESS + self.overlay.unknown_start
        self.assertEqual(
            (self.output / f"unknown/{address:08X}.bin").read_bytes(),
            b"\x00\x00" + bytes.fromhex("12 34 56 78"),
        )

    def test_nonzero_runtime_bytes_are_saved(self):
        self.overlay.data[-1] = 0x5A
        cload.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "unknown/variables.bin").read_bytes(), bytes(31) + b"Z")

    def test_missing_symbol_reports_what_to_update(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_cload_location_names")
        with self.assertRaisesRegex(ValueError, "g_cload_location_names.*SYMBOL_NAMES"):
            cload.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_blob_reports_make_splat(self):
        inputs = self.overlay.write(self.root)
        (inputs.assets / "blob.databin.bin").unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            cload.extract(inputs, self.output)
        self.assert_no_output()

    def test_truncated_blob_writes_nothing(self):
        inputs = self.overlay.write(self.root)
        (inputs.assets / "blob.databin.bin").write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            cload.extract(inputs, self.output)
        self.assert_no_output()

    def test_out_of_order_symbols_are_rejected(self):
        self.overlay.symbols[cload.SYMBOL_NAMES["locations"]] = ADDRESS
        with self.assertRaisesRegex(ValueError, "g_cload_location_names is out of resource order"):
            cload.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_sequence_cannot_run_into_the_chart(self):
        self.overlay.data[self.overlay.steps_end - 4 : self.overlay.steps_end] = bytes([14]) * 4
        with self.assertRaisesRegex(ValueError, "g_cload_steps_idle has no CLOAD_STEP_DONE"):
            cload.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_step_symbol_outside_the_table_is_rejected(self):
        self.overlay.symbols["g_cload_steps_idle"] = ADDRESS
        with self.assertRaisesRegex(ValueError, "g_cload_steps_idle is outside"):
            cload.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_write_failure_removes_staging(self):
        with patch.object(cload, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                cload.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_existing_output_is_preserved(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            cload.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
