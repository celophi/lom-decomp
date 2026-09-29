"""Run the ADDHERO extractor end to end on a small synthetic overlay.

The fake overlay has one of everything the real blob holds, laid out in the
same order, so a change to how the extractor walks the blob shows up here
without any game files.
"""

import struct
import tempfile
import unittest
from pathlib import Path

import yaml

from tools.overlays import addhero, card_data, icon_set

ADDRESS = 0x80100000  # where the fake overlay's first data file sits in memory
FULL_WIDTH_A = "Ａ".encode("shift_jis")


def text_table(strings: list[bytes]) -> bytes:
    body = b""
    offsets = []
    for string in strings:
        offsets.append(len(strings) * 2 + len(body))
        body += string + b"\x00"
    return b"".join(struct.pack("<H", offset) for offset in offsets) + body


def pad(data: bytearray, alignment: int = 4) -> None:
    data.extend(b"\x00" * (-len(data) % alignment))


class FakeOverlay:
    """Builds the three data files, their splat config and a symbol file."""

    def __init__(self):
        small = "ＭＡＸ".encode("shift_jis") + b"\x00\x00" + b"bu00:\x00\x00\x00"
        pattern = b"bu00:*\x00\x00"
        blob = bytearray()
        names = {}

        names["g_addhero_text_table"] = len(blob)
        names["g_addhero_text_no_card"] = len(blob) + 2  # entry 1
        blob += text_table([b"Load?", b"No card"])
        for title in ("ＭＡＮＡ", "ＢＡＤ"):
            pad(blob)
            blob += title.encode("shift_jis") + b"\x00"
        pad(blob)
        names["g_addhero_location_text_table"] = len(blob)
        blob += text_table([b"Home"])
        pad(blob)

        palette = struct.pack("<16H", 0, *range(1, 16))
        icon = palette + bytes(index % 256 for index in range(icon_set.PIXEL_BYTES))
        names["g_addhero_icon_image_table"] = len(blob) + 4
        blob += struct.pack("<II", 1, 8) + icon
        blob += blob[-4:]  # the repeated trailing word

        names["g_addhero_loadseq_start"] = len(blob)
        blob += bytes([1, 2, 0, 0])
        names["g_card_steps_idle"] = len(blob)
        blob += bytes([14, 0, 0, 0])

        names["g_glyph_single_byte_chart"] = len(blob)
        row = FULL_WIDTH_A * card_data.CHART_COLUMNS + b"\n"
        blob += row * (0x10 - (card_data.CHART_FIRST_CODE >> 4))
        pad(blob)
        # Put the two-byte pages past the chart, so no page falls inside it.
        names["g_glyph_chart_page_base"] = 0x10000

        names["g_glyph_decimal_digits"] = len(blob)
        blob += "０１２３４５６７８９".encode("shift_jis") + b"\x00\x00"
        pad(blob, 8)
        names["g_glyph_hex_digits"] = len(blob)
        blob += "０１２３４５６７８９ＡＢＣＤＥＦ".encode("shift_jis") + b"\x00\x00"
        pad(blob, 8)
        names["g_addhero_icon_phase"] = len(blob)  # first variable
        blob += bytes(32)

        self.small, self.pattern, self.blob = small, pattern, bytes(blob)
        self.blob_address = ADDRESS + len(small) + len(pattern)
        self.symbols = {name: self.blob_address + offset for name, offset in names.items()}
        page_base = names["g_glyph_chart_page_base"]
        self.symbols["g_glyph_chart_page_base"] = self.blob_address + page_base
        self.symbols["g_decimal_overflow_text"] = ADDRESS
        self.symbols["g_addhero_file_template"] = ADDRESS + 8
        self.symbols["g_addhero_entry_header_template"] = ADDRESS + len(small)

    def write(self, root: Path, skip_symbol: str | None = None) -> addhero.Inputs:
        config, assets = root / "config", root / "assets"
        (config / "symbols").mkdir(parents=True)
        (config / "overlays").mkdir()
        assets.mkdir()
        (assets / "small.rodatabin.bin").write_bytes(self.small)
        (assets / "pattern.rodatabin.bin").write_bytes(self.pattern)
        (assets / "blob.databin.bin").write_bytes(self.blob)
        start = 0x1
        segment = {
            "name": "addhero",
            "type": "decompress_overlay",
            "start": start,
            "vram": ADDRESS,
            "subsegments": [
                [start, "rodatabin", "small"],
                [start + len(self.small), "rodatabin", "pattern"],
                [start + len(self.small) + len(self.pattern), "databin", "blob"],
            ],
        }
        end = start + len(self.small) + len(self.pattern) + len(self.blob)
        (config / addhero.OVERLAY_CONFIG).write_text(yaml.safe_dump({"segments": [segment, [end]]}))
        lines = [
            f"{name} = 0x{address:08X};"
            for name, address in self.symbols.items()
            if name != skip_symbol
        ]
        (config / addhero.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return addhero.Inputs("us", config, assets)


class ExtractTest(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.root = Path(self.folder.name)
        self.overlay = FakeOverlay()

    def tearDown(self):
        self.folder.cleanup()

    def load(self, path: str) -> dict:
        return yaml.safe_load((self.root / "out" / path).read_text(encoding="utf-8"))

    def test_writes_every_part(self):
        addhero.extract(self.overlay.write(self.root), self.root / "out")
        messages = self.load("text/messages.yaml")["entries"]
        self.assertEqual([entry["text"] for entry in messages], ["Load?", "No card"])
        self.assertEqual(messages[1]["symbol"], "g_addhero_text_no_card")
        titles = self.load("text/card_titles.yaml")["titles"]
        self.assertEqual([title["text"] for title in titles], ["ＭＡＮＡ", "ＢＡＤ"])
        self.assertEqual(self.load("text/locations.yaml")["entries"][0]["text"], "Home")
        self.assertTrue((self.root / "out/icons/icon_00.png").exists())
        steps = self.load("tables/card_steps.yaml")["sequences"]
        self.assertEqual(steps[1]["steps"], ["ADDHERO_STEP_WAIT", "ADDHERO_STEP_DONE"])
        self.assertEqual(self.load("tables/text_conversion.yaml")["one_byte"]["0x4_"], "Ａ" * 16)
        self.assertEqual(len(self.load("tables/digit_glyphs.yaml")["hexadecimal"]["glyphs"]), 16)
        strings = [entry["text"] for entry in self.load("text/fixed_strings.yaml")["strings"]]
        self.assertEqual(strings, ["ＭＡＸ", "bu00:", "bu00:*"])

    def test_byte_map_covers_the_whole_blob_in_order(self):
        addhero.extract(self.overlay.write(self.root), self.root / "out")
        ranges = self.load("byte-map.yaml")["ranges"]
        cursor = 0
        for entry in ranges:
            self.assertEqual(int(entry["offset"], 16), cursor, entry["name"])
            cursor += int(entry["size"], 16)
        self.assertEqual(cursor, len(self.overlay.blob))
        names = [entry["name"] for entry in ranges]
        self.assertIn("icon set trailing word", names)
        self.assertEqual(names[-1], "variables")
        self.assertNotIn("unknown", names)

    def test_a_missing_symbol_names_itself_and_writes_nothing(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_addhero_icon_image_table")
        with self.assertRaisesRegex(ValueError, "g_addhero_icon_image_table.*SYMBOL_NAMES"):
            addhero.extract(inputs, self.root / "out")
        self.assertEqual(list(self.root.glob("out*")) + list(self.root.glob(".out*")), [])

    def test_refuses_an_existing_folder(self):
        (self.root / "out").mkdir()
        with self.assertRaises(FileExistsError):
            addhero.extract(self.overlay.write(self.root), self.root / "out")


if __name__ == "__main__":
    unittest.main()
