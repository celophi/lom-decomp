"""Exercise SHOP's export using invented data, without game files."""

import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.overlays import shop
from tools.overlays.tests.test_cload_extract import FakeOverlay as FakeCload
from tools.overlays.tests.test_overlay_tools import make_table

ADDRESS = 0x80142D04
KINDS = shop.ITEM_KIND_COUNT


class FakeOverlay:
    """A four-section archive, a price per item kind and a little runtime state."""

    def __init__(self, version="us"):
        self.version = version
        self.symbols = {}
        self.data = bytearray()

        def mark(key):
            self.symbols[shop.SYMBOL_NAMES[key]] = ADDRESS + len(self.data)

        mark("text")
        if version == "us":
            descriptions = [b"ORE\x08Primary\x0b\x0f"] + [b""] * (KINDS - 1)
            names = [b"Tin", b"R\x16g"] + [b"Item"] * (KINDS - 2)
        else:
            descriptions = [b"\x41\x19\x00"] + [b""] * (KINDS - 1)
            names = [b"\x41", b"\x41\x41"] + [b""] * (KINDS - 2)
        types = [b"Type%d" % index for index in range(27)] + [b""] * 5
        spells = [b"Spell%d" % index for index in range(2 * shop.SPELLS_PER_SPIRIT)]
        sections = [make_table(descriptions), make_table(names) + bytes(2), make_table(types), make_table(spells)]
        offsets = []
        cursor = shop.ARCHIVE_HEADER.size
        for section in sections:
            offsets.append(cursor)
            cursor += len(section)
        self.archive = shop.ARCHIVE_HEADER.pack(4, 0, *offsets) + b"".join(sections)
        self.archive += bytes(4 - len(self.archive) % 4)  # always some alignment padding in the last section
        self.data += self.archive
        mark("prices")
        self.prices = struct.pack(f"<{KINDS}H", *(index * 10 for index in range(KINDS)))
        self.data += self.prices + bytes(4)
        mark("variables")
        self.data += bytes(32)

    def offset(self, key):
        return self.symbols[shop.SYMBOL_NAMES[key]] - ADDRESS

    def write(self, root, skip_symbol=None):
        config, assets = root / "config", root / "assets"
        if self.version == "jp":
            FakeCload("jp").write(root)
        else:
            (config / "symbols").mkdir(parents=True)
            (config / "overlays").mkdir()
            assets.mkdir()
        (assets / "shop_data.databin.bin").write_bytes(self.data)
        segment = {"start": 1, "vram": ADDRESS, "subsegments": [[1, "databin", "shop_data"]]}
        (config / shop.OVERLAY_CONFIG).write_text(yaml.safe_dump({"segments": [segment, [1 + len(self.data)]]}))
        lines = [f"{name} = 0x{address:08X};" for name, address in self.symbols.items() if name != skip_symbol]
        (config / shop.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return shop.Inputs(self.version, config, assets)


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

    def test_text_keeps_indices_codes_and_complete_archive(self):
        shop.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "text/archive.bin").read_bytes(), self.overlay.archive)
        archive = self.load("text/archive.yaml")
        self.assertEqual([section["file"] for section in archive["sections"]],
                         ["item_descriptions.yaml", "item_names.yaml", "equipment_types.yaml", "instrument_spells.yaml"])
        description = self.load("text/item_descriptions.yaml")["entries"][0]
        self.assertEqual(description["text"], "ORE{08}Primary{0B}{0F}")
        self.assertEqual(description["bytes"], "4f 52 45 08 50 72 69 6d 61 72 79 0b 0f")
        names = self.load("text/item_names.yaml")
        self.assertEqual(len(names["entries"]), KINDS)
        self.assertEqual(names["entries"][1]["text"], "R{16}g")
        self.assertEqual(names["padding"], 2)
        self.assertNotIn("padding", self.load("text/equipment_types.yaml"))
        self.assertGreater(self.load("text/instrument_spells.yaml")["padding"], 0)

    def test_entries_carry_the_indices_the_renderer_uses(self):
        shop.extract(self.overlay.write(self.root), self.output)
        names = self.load("text/item_names.yaml")["entries"]
        self.assertTrue(names[shop.MATERIAL_COUNT - 1]["material"])
        self.assertFalse(names[shop.MATERIAL_COUNT]["material"])
        types = self.load("text/equipment_types.yaml")["entries"]
        self.assertEqual({key: types[10][key] for key in ("category", "type")}, {"category": "weapon", "type": 10})
        self.assertEqual({key: types[11][key] for key in ("category", "type")}, {"category": "armor", "type": 0})
        self.assertEqual({key: types[26][key] for key in ("category", "type", "text")},
                         {"category": "instrument", "type": 3, "text": "Type26"})
        self.assertEqual(types[31]["text"], "")
        spells = self.load("text/instrument_spells.yaml")["entries"]
        self.assertEqual((spells[15]["spirit"], spells[15]["spell"], spells[15]["text"]), (1, 1, "Spell15"))

    def test_japanese_text_uses_cload_and_keeps_zero_second_bytes(self):
        shop.extract(FakeOverlay("jp").write(self.root), self.output)
        entry = self.load("text/item_descriptions.yaml")["entries"][0]
        self.assertEqual(entry["text"], "\uff21\uff21")
        self.assertEqual(entry["bytes"], "41 19 00")
        prices = self.load("tables/sell_prices.yaml")["entries"]
        self.assertEqual(prices[1]["name"], "\uff21\uff21")
        self.assertEqual(self.load("byte-map.yaml")["text_decoder"]["overlay"], "CLOAD")

    def test_sell_prices_name_each_item_kind(self):
        shop.extract(self.overlay.write(self.root), self.output)
        prices = self.load("tables/sell_prices.yaml")
        self.assertEqual(len(prices["entries"]), KINDS)
        self.assertEqual(prices["entries"][2], {"item": 2, "name": "Item", "price": 20})
        self.assertEqual(prices["entries"][0]["name"], "Tin")
        self.assertEqual(bytes.fromhex(prices["bytes"]), self.overlay.prices)

    def test_byte_map_covers_every_byte_including_runtime_state(self):
        shop.extract(self.overlay.write(self.root), self.output)
        cursor = 0
        parts = self.load("byte-map.yaml")["ranges"]
        for part in parts:
            self.assertEqual(int(part["offset"], 16), cursor)
            cursor += int(part["size"], 16)
            if "file" in part:
                self.assertTrue((self.output / part["file"]).is_file())
        self.assertEqual(cursor, len(self.overlay.data))
        self.assertEqual([part["name"] for part in parts], ["text archive", "sell prices", "padding", "shop state"])
        self.assertNotIn("file", parts[-1])
        self.assertFalse((self.output / "unknown").exists())

    def test_nonzero_gap_and_runtime_state_are_preserved(self):
        self.overlay.data[self.overlay.offset("variables") - 1] = 0x77
        self.overlay.data[-1] = 0x5A
        shop.extract(self.overlay.write(self.root), self.output)
        gap = self.overlay.symbols["g_shop_title_text_id"] - 4
        self.assertEqual((self.output / f"unknown/{gap:08X}.bin").read_bytes(), bytes(3) + b"\x77")
        self.assertEqual((self.output / "unknown/variables.bin").read_bytes(), bytes(31) + b"Z")

    def test_missing_symbol_reports_what_to_update(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_shop_item_sell_prices")
        with self.assertRaisesRegex(ValueError, "g_shop_item_sell_prices.*SYMBOL_NAMES"):
            shop.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_or_truncated_blob_writes_nothing(self):
        inputs = self.overlay.write(self.root)
        path = inputs.assets / "shop_data.databin.bin"
        path.write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            shop.extract(inputs, self.output)
        path.unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            shop.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_japanese_chart_writes_nothing(self):
        inputs = FakeOverlay("jp").write(self.root)
        (inputs.assets / "blob.databin.bin").unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            shop.extract(inputs, self.output)
        self.assert_no_output()

    def test_wrong_resource_order_is_rejected(self):
        self.overlay.symbols[shop.SYMBOL_NAMES["prices"]] = ADDRESS
        with self.assertRaisesRegex(ValueError, "out of resource order"):
            shop.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_section_offsets_must_stay_inside_the_archive(self):
        struct.pack_into("<I", self.overlay.data, 16, len(self.overlay.data))
        with self.assertRaisesRegex(ValueError, "section offsets"):
            shop.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_text_string_cannot_read_the_next_section(self):
        start = struct.unpack_from("<I", self.overlay.data, 12)[0]
        struct.pack_into("<H", self.overlay.data, start + 2, 0xFFFF)
        with self.assertRaisesRegex(ValueError, "invalid offset"):
            shop.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_item_sections_need_one_entry_per_item_kind(self):
        start = struct.unpack_from("<I", self.overlay.data, 8)[0]
        struct.pack_into("<H", self.overlay.data, start, (KINDS - 1) * 2)
        with self.assertRaisesRegex(ValueError, "item_names must have 256 entries"):
            shop.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_short_price_table_is_rejected(self):
        self.overlay.symbols[shop.SYMBOL_NAMES["variables"]] -= 8
        with self.assertRaisesRegex(ValueError, "sell price table"):
            shop.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_failed_write_removes_staging(self):
        with patch.object(shop, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                shop.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_existing_output_is_preserved(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            shop.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
