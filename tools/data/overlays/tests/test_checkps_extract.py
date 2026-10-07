"""Run CHECKPS's extractor on a small made-up overlay, without game files.

The fixture has both AKAO sections, two image palettes, the CD tables and the
separate warning rodata. Unidentified trailing bytes are deliberate: the byte
map must preserve them along with the resources we understand.
"""

import struct
import tempfile
import unittest
import zlib
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.data.formats.psx_tim import TimBlock, TimImage
from tools.data.overlays import checkps

ADDRESS = 0x80100000


class FakeOverlay:
    """Build the resources and their symbol/config files in memory."""

    def __init__(self, version="us"):
        self.version = version
        self.symbols = {}
        self.warning = bytes.fromhex("82 60 0a 82 61 0a 82 62")
        self.rodata = self.warning.ljust(checkps.WARNING_BYTES, b"\x00")
        self.rodata += struct.pack("<8b", -1, -1, 1, -1, 1, 1, -1, 1) + bytes(12)
        self.symbols[checkps.SYMBOL_NAMES["warning"]] = ADDRESS
        self.symbols[checkps.SYMBOL_NAMES["quadrant_signs"]] = ADDRESS + checkps.WARNING_BYTES
        self.address = ADDRESS + len(self.rodata) + 32  # stand-in for the code
        data = bytearray()

        def mark(key):
            self.symbols[checkps.SYMBOL_NAMES[key]] = self.address + len(data)

        mark("audio")
        self.program = b"AKAO" + bytes(12) + b"program!"
        bank_offset = 12 + len(self.program)
        self.bank = bytearray(b"AKAO" + bytes(12))
        self.bank += struct.pack("<4I", 0x70000, 32, 5, 1) + bytes(32)
        self.bank += struct.pack("<4I", 0, 16, 0x480000, 0x5FC700FF)
        self.samples = bytes(range(32))
        self.bank += self.samples
        data += struct.pack("<III", 2, 12, bank_offset) + self.program + self.bank
        self.audio_end = len(data)
        data += data[-4:]
        data += b"tail"
        mark("image")
        palette = struct.pack("<16H", 0, 0x001F, 0x03E0, 0x7C00, *([0x7FFF] * 12))
        alternate = struct.pack("<16H", 0, 0x7C00, 0x001F, 0x03E0, *([0x7FFF] * 12))
        self.tim = TimImage(8, TimBlock(0, 480, 16, 2, palette + alternate),
                            TimBlock(320, 0, 1, 1, b"\x21\x03"))
        data += self.tim.to_bytes()
        self.image_end = len(data)
        data += data[-4:]
        data += b"unused!!"
        mark("commands")
        commands = checkps.enum_names("CheckPSCdCommandIndex")
        data += b"".join(bytes([index, 0, 1, 3]) for index in commands)
        for key, address in zip(
            ("status_register", "response_register", "data_register", "irq_register"),
            range(0x1F801800, 0x1F801804),
        ):
            mark(key)
            data += struct.pack("<I", address)
        mark("parameters")
        data += bytes(8)
        mark("response")
        data += bytes(8)
        mark("state")
        data += struct.pack("<I", 1)
        mark("irq_sum")
        data += bytes(4)
        mark("pattern_sizes")
        data += bytes(range(checkps.PATTERN_SIZE_COUNT * 2)) + bytes(6)
        mark("decimal_glyphs")
        data += b"".join(bytes([0x82, 0x4F + i]) for i in range(10)) + bytes(4)
        mark("hex_glyphs")
        data += b"".join(bytes([0x82, 0x4F + i]) for i in range(10))
        data += b"".join(bytes([0x82, 0x60 + i]) for i in range(6)) + bytes(4)
        mark("clut_prefix")
        data += struct.pack("<6H", 0, 0xFFFF, 0xBDEF, 0, 0, 0)
        mark("bss")
        self.data = data

    def write(self, root: Path, skip_symbol=None) -> checkps.Inputs:
        config, assets = root / "config", root / "assets"
        (config / "symbols").mkdir(parents=True)
        (config / "overlays").mkdir()
        assets.mkdir()
        (assets / "warning.rodatabin.bin").write_bytes(self.rodata)
        (assets / "blob.databin.bin").write_bytes(self.data)
        start = self.address - ADDRESS + 1
        end = start + len(self.data)
        segment = {
            "name": "checkps", "type": "code", "start": 1, "vram": ADDRESS,
            "subsegments": [
                [1, "rodatabin", "warning"], [1 + len(self.rodata), "c", "stub"],
                [start, "databin", "blob"], [end, "bss", "stub"],
            ],
        }
        (config / checkps.OVERLAY_CONFIG).write_text(yaml.safe_dump({"segments": [segment, [end + 32]]}))
        lines = [f"{name} = 0x{address:08X};" for name, address in self.symbols.items() if name != skip_symbol]
        (config / checkps.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return checkps.Inputs(self.version, config, assets)


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

    def test_writes_audio_image_warning_and_tables(self):
        checkps.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "audio/program.akao").read_bytes(), self.overlay.program)
        self.assertEqual((self.output / "audio/bank.akao").read_bytes(), self.overlay.bank)
        self.assertEqual((self.output / "audio/samples.adpcm").read_bytes(), self.overlay.samples)
        bank = self.load("audio/bank.yaml")
        self.assertEqual(bank["sample_size"], 32)
        self.assertEqual(bank["articulations"][0]["index"], 5)
        self.assertEqual(bank["articulations"][0]["loop_offset"], "0x10")
        self.assertEqual((self.output / "image/image.tim").read_bytes(), self.overlay.tim.to_bytes())
        self.assertEqual(len(self.load("image/image.yaml")["palettes"]), 2)
        warning = self.load("text/warning.yaml")
        self.assertEqual(warning["lines"], ["\uff21", "\uff22", "\uff23"])
        self.assertEqual(len(bytes.fromhex(warning["bytes"])), 60)
        self.assertEqual(self.load("tables/pattern_signs.yaml")["signs"][0], {"x": -1, "y": -1})
        self.assertEqual(len(self.load("tables/pattern_sizes.yaml")["sizes"]), 17)
        self.assertEqual(self.load("tables/cd_commands.yaml")["commands"][0]["name"], "CHECKPS_CD_CMD_NOP")
        self.assertEqual(self.load("tables/cd_registers.yaml")["registers"][0]["value"], "0x1F801800")
        self.assertEqual(self.load("tables/cd_state.yaml")["state_name"], "CHECKPS_STATE_START_GET_TN")
        digits = self.load("tables/digit_glyphs.yaml")
        self.assertEqual(len(digits["decimal"]["glyphs"]), 10)
        self.assertEqual(len(digits["hexadecimal"]["glyphs"]), 16)
        self.assertEqual(self.load("tables/glyph_clut.yaml")["colors"][:3], ["0x0000", "0xFFFF", "0xBDEF"])

    def test_pngs_use_each_palette_and_low_nibble_first(self):
        checkps.extract(FakeOverlay("jp").write(self.root), self.output)
        expected = (
            bytes([255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 0, 0]),
            bytes([0, 0, 255, 255, 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0, 0]),
        )
        for index, pixels in enumerate(expected):
            raw = (self.output / f"image/palette_{index:02X}.png").read_bytes()
            self.assertEqual(struct.unpack(">II", raw[16:24]), (4, 1))
            start = raw.index(b"IDAT") + 4
            length = struct.unpack_from(">I", raw, start - 8)[0]
            self.assertEqual(zlib.decompress(raw[start : start + length]), b"\x00" + pixels)

    def test_both_byte_maps_cover_their_sources_once_and_preserve_unknown_bytes(self):
        checkps.extract(self.overlay.write(self.root), self.output)
        data = self.load("byte-map.yaml")
        for source, expected in ((data, self.overlay.data), (data["other_files"][0], self.overlay.rodata)):
            cursor = 0
            for part in source["ranges"]:
                self.assertEqual(int(part["offset"], 16), cursor, part["name"])
                size = int(part["size"], 16)
                if part["name"] == "unknown":
                    self.assertEqual((self.output / part["file"]).read_bytes(), expected[cursor:cursor + size])
                cursor += size
            self.assertEqual(cursor, len(expected))
        self.assertEqual(len(list((self.output / "unknown").glob("*.bin"))), 2)
        names = [p["name"] for p in data["ranges"]]
        self.assertIn("AKAO trailing word", names)
        self.assertIn("TIM trailing word", names)
        self.assertEqual(int(data["bss"]["address"], 16), self.overlay.address + len(self.overlay.data))

    def test_bad_audio_offsets_write_nothing(self):
        struct.pack_into("<I", self.overlay.data, 8, len(self.overlay.data))
        with self.assertRaisesRegex(ValueError, "invalid CHECKPS AKAO section offsets"):
            checkps.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_audio_samples_cannot_read_the_image(self):
        bank = struct.unpack_from("<I", self.overlay.data, 8)[0]
        struct.pack_into("<I", self.overlay.data, bank + 20, len(self.overlay.data))
        with self.assertRaisesRegex(ValueError, "samples run past the next resource"):
            checkps.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_image_block_cannot_read_the_cd_tables(self):
        image = self.overlay.symbols[checkps.SYMBOL_NAMES["image"]] - self.overlay.address
        struct.pack_into("<I", self.overlay.data, image + 8, len(self.overlay.data))
        with self.assertRaisesRegex(ValueError, "past file size"):
            checkps.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_missing_symbol_reports_its_name(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_cd_command_table")
        with self.assertRaisesRegex(ValueError, "g_cd_command_table.*SYMBOL_NAMES"):
            checkps.extract(inputs, self.output)
        self.assert_no_output()

    def test_truncated_data_is_rejected(self):
        inputs = self.overlay.write(self.root)
        (inputs.assets / "blob.databin.bin").write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            checkps.extract(inputs, self.output)
        self.assert_no_output()

    def test_write_failure_removes_staging(self):
        with patch.object(checkps, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                checkps.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_existing_output_is_preserved(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            checkps.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
