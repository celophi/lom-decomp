"""Exercise GOLEM's export using invented data, without game files."""

import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.data.formats.psx_tim import TimBlock, TimImage
from tools.data.overlays import golem
from tools.data.overlays.tests.test_cload_extract import FakeOverlay as FakeCload
from tools.data.overlays.tests.test_field_extract import png_pixels
from tools.data.overlays.tests.test_overlay_tools import make_table

ADDRESS = 0x8014287C


class FakeOverlay:
    """A tiny atlas and text archive, plus the full fixed record arrays."""

    def __init__(self, version="us"):
        self.version = version
        self.symbols = {}
        self.data = bytearray()

        def mark(key):
            self.symbols[golem.SYMBOL_NAMES[key]] = ADDRESS + len(self.data)

        mark("image")
        palettes = [struct.pack("<16H", 0, color, 0x03E0, 0x7C00, *([0x7FFF] * 12))
                    for color in (0x001F, 0x03E0, 0x7C00, 0x7FFF, 0x8000)]
        self.tim = TimImage(8, TimBlock(0, 480, 16, 5, b"".join(palettes)),
                           TimBlock(0, 0, 1, 2, bytes.fromhex("10 32 23 01")))
        self.data += self.tim.to_bytes()
        self.data += self.data[-4:]
        mark("text")
        names = make_table([b"Block", b"Block"] if version == "us" else [b"\x41\x19\x00", b"\x41"])
        descriptions = make_table([b"Stay\x1f\x00", b""] if version == "us" else [b"\x19\x00", b""])
        self.archive = struct.pack("<3I", 2, 12, 12 + len(names)) + names + descriptions + bytes(3)
        self.data += self.archive
        mark("glyphs")
        for index in range(82):
            self.data += bytes(8) if index in (0, 20, 53) else struct.pack("<4B2H", 1, 5, 0, 6, 2, 2)
        mark("panels")
        attributes = 3 | 4 | (6 << 3) | (9 << 7) | (214 << 11) | (1 << 25)
        texture = 3 | (167 << 3) | (10 << 11) | (257 << 17) | (5 << 26)
        self.panel = struct.pack("<II6H", attributes, texture, 0x125, 400, 600, 511, 800, 0xABCD)
        self.data += self.panel * 154
        mark("variables")
        self.data += bytes(32)

    def offset(self, key):
        return self.symbols[golem.SYMBOL_NAMES[key]] - ADDRESS

    def write(self, root, skip_symbol=None):
        config, assets = root / "config", root / "assets"
        if self.version == "jp":
            FakeCload("jp").write(root)
        else:
            (config / "symbols").mkdir(parents=True)
            (config / "overlays").mkdir()
            assets.mkdir()
        (assets / "golem_data.databin.bin").write_bytes(self.data)
        segment = {"start": 1, "vram": ADDRESS, "subsegments": [[1, "databin", "golem_data"]]}
        (config / golem.OVERLAY_CONFIG).write_text(yaml.safe_dump({"segments": [segment, [1 + len(self.data)]]}))
        lines = [f"{name} = 0x{address:08X};" for name, address in self.symbols.items() if name != skip_symbol]
        (config / golem.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return golem.Inputs(self.version, config, assets)


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
        golem.extract(self.overlay.write(self.root), self.output)
        names = self.load("text/names.yaml")["entries"]
        self.assertEqual([entry["text"] for entry in names], ["Block", "Block"])
        self.assertEqual([entry["index"] for entry in names], [0, 1])
        descriptions = self.load("text/descriptions.yaml")["entries"]
        self.assertEqual(descriptions[0]["text"], "Stay{1F 00}")
        self.assertEqual(descriptions[0]["bytes"], "53 74 61 79 1f 00")
        self.assertEqual(descriptions[1]["text"], "")
        self.assertEqual((self.output / "text/archive.bin").read_bytes(), self.overlay.archive)

    def test_japanese_text_uses_cload_and_keeps_zero_second_bytes(self):
        golem.extract(FakeOverlay("jp").write(self.root), self.output)
        entry = self.load("text/names.yaml")["entries"][0]
        self.assertEqual(entry["text"], "\uff21\uff21")
        self.assertEqual(entry["bytes"], "41 19 00")
        self.assertEqual(self.load("byte-map.yaml")["text_decoder"]["overlay"], "CLOAD")

    def test_texture_and_glyph_previews_keep_palette_order(self):
        golem.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "image/image.tim").read_bytes(), self.overlay.tim.to_bytes())
        width, height, rows = png_pixels(self.output / "image/palette_00.png")
        self.assertEqual((width, height), (4, 2))
        self.assertEqual(rows[:9], bytes([0, 0, 0, 0, 0, 255, 0, 0, 255]))
        width, height, rows = png_pixels(self.output / "image/glyphs/01.png")
        self.assertEqual((width, height), (8, 4))

        def pixel(x, y):
            start = y * (width * 4 + 1) + 1 + x * 4
            return tuple(rows[start:start + 4])

        self.assertEqual(pixel(0, 0), (255, 0, 0, 255))
        self.assertEqual(pixel(2, 0), (0, 255, 0, 255))
        self.assertEqual(pixel(4, 0), (0, 0, 255, 255))
        self.assertEqual(pixel(6, 0), (255, 255, 255, 255))
        self.assertEqual(pixel(0, 2), (0, 0, 0, 255))  # 0x8000 is opaque black in previews.
        self.assertEqual(pixel(2, 2), (0, 0, 0, 0))  # Unused preview cell.
        self.assertEqual(pixel(0, 1), (0, 255, 0, 255))
        glyphs = self.load("image/image.yaml")["glyphs"]
        self.assertEqual(len(glyphs), 82)
        self.assertNotIn("file", glyphs[0])
        self.assertNotIn("file", glyphs[20])
        self.assertNotIn("file", glyphs[53])
        self.assertEqual(len(list((self.output / "image/glyphs").glob("*.png"))), 79)

    def test_panels_decode_height_bits_flags_and_reserved_fields(self):
        golem.extract(self.overlay.write(self.root), self.output)
        panels = self.load("tables/panels.yaml")
        entry = panels["entries"][0]
        expected = {
            "blend_mode": 3, "semi_transparent": True, "behavior": 6,
            "behavior_name": "GOLEM_PANEL_FLASH_WHITE", "flash_frames": 9,
            "u": 214, "v": 167, "palette": 10, "cell_width": 257, "cell_height": 325,
            "x": 400, "y": 600, "width": 511, "height": 800,
            "texture_height_high": 0x125, "reserved": 0xABCD,
        }
        self.assertEqual({key: entry[key] for key in expected}, expected)
        self.assertEqual(bytes.fromhex(panels["bytes"]), self.overlay.panel * 154)
        glyph = self.load("tables/glyph_metrics.yaml")["entries"][1]
        self.assertEqual((glyph["reserved_01"], glyph["reserved_03"]), (5, 6))

    def test_byte_map_covers_every_byte_including_runtime_state(self):
        golem.extract(self.overlay.write(self.root), self.output)
        cursor = 0
        parts = self.load("byte-map.yaml")["ranges"]
        for part in parts:
            self.assertEqual(int(part["offset"], 16), cursor)
            cursor += int(part["size"], 16)
            if "file" in part:
                self.assertTrue((self.output / part["file"]).is_file())
        self.assertEqual(cursor, len(self.overlay.data))
        self.assertEqual(parts[-1]["name"], "editor state")
        self.assertNotIn("file", parts[-1])
        self.assertFalse((self.output / "unknown").exists())

    def test_nonzero_runtime_state_is_preserved(self):
        self.overlay.data[-1] = 0x5A
        golem.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "unknown/variables.bin").read_bytes(), bytes(31) + b"Z")

    def test_missing_symbol_reports_what_to_update(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_golem_panel_records")
        with self.assertRaisesRegex(ValueError, "g_golem_panel_records.*SYMBOL_NAMES"):
            golem.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_or_truncated_blob_writes_nothing(self):
        inputs = self.overlay.write(self.root)
        path = inputs.assets / "golem_data.databin.bin"
        path.write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            golem.extract(inputs, self.output)
        path.unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            golem.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_japanese_chart_writes_nothing(self):
        inputs = FakeOverlay("jp").write(self.root)
        (inputs.assets / "blob.databin.bin").unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            golem.extract(inputs, self.output)
        self.assert_no_output()

    def test_wrong_resource_order_is_rejected(self):
        self.overlay.symbols[golem.SYMBOL_NAMES["text"]] = ADDRESS
        with self.assertRaisesRegex(ValueError, "out of resource order"):
            golem.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_text_section_cannot_extend_into_the_glyph_table(self):
        struct.pack_into("<I", self.overlay.data, self.overlay.offset("text") + 8, len(self.overlay.archive) + 1)
        with self.assertRaisesRegex(ValueError, "section offsets"):
            golem.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_text_string_cannot_read_the_next_section(self):
        struct.pack_into("<H", self.overlay.data, self.overlay.offset("text") + 14, 0xFFFF)
        with self.assertRaisesRegex(ValueError, "invalid offset"):
            golem.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_incomplete_glyph_or_panel_table_is_rejected(self):
        self.overlay.symbols[golem.SYMBOL_NAMES["variables"]] -= 4
        with self.assertRaisesRegex(ValueError, "complete panel records"):
            golem.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_invalid_glyph_rectangle_writes_nothing(self):
        self.overlay.data[self.overlay.offset("glyphs") + 8] = 255
        with self.assertRaisesRegex(ValueError, "glyph 1 lies outside"):
            golem.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_tims_trailing_word_must_really_be_a_copy(self):
        self.overlay.data[self.overlay.offset("text") - 1] ^= 1
        with self.assertRaisesRegex(ValueError, "does not duplicate"):
            golem.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_failed_write_removes_staging(self):
        with patch.object(golem, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                golem.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_existing_output_is_preserved(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            golem.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
