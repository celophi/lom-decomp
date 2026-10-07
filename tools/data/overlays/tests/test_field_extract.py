"""Run FIELD's extractor on a small synthetic overlay, without disc data."""

import shutil
import struct
import tempfile
import unittest
import zlib
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.data.formats.psx_tim import TimBlock, TimImage
from tools.data.overlays import field
from tools.data.overlays.tests.test_cload_extract import FakeOverlay as FakeCload
from tools.data.overlays.tests.test_overlay_tools import make_table

ADDRESS = 0x800C0000
PALETTE = struct.pack("<16H", 0, 0x001F, 0x03E0, 0x7C00, *([0x7FFF] * 12))


class FakeOverlay:
    """All the resource shapes FIELD reads, with short text and animation payloads."""

    def __init__(self, version="us"):
        self.version = version
        self.symbols = {"test_handler": 0x80012340}
        self.data = bytearray()
        specs = {spec.symbol: spec for spec in field.TABLES}

        def mark(key):
            self.symbols[field.SYMBOL_NAMES[key]] = ADDRESS + len(self.data)

        def add_table(symbol):
            spec = specs[symbol]
            self.symbols[symbol] = ADDRESS + len(self.data)
            raw = bytearray(struct.calcsize("<" + spec.format) * spec.count)
            if spec.resolve_symbols:
                struct.pack_into("<I", raw, 0, self.symbols["test_handler"])
            self.data.extend(raw)

        def pad():
            self.data.extend(bytes(-len(self.data) % 4))

        def add_text(key, count=1):
            mark(key)
            text = b"Test" if version == "us" else b"\x41\x19\x00"
            self.data.extend(make_table([text] * count))
            pad()

        add_table(field.SYMBOL_NAMES["pixels"])
        self.unknown_start = len(self.data)
        self.data.extend(b"test")
        mark("texture")
        self.tim = TimImage(8, TimBlock(0, 480, 16, 1, PALETTE), TimBlock(320, 0, 1, 1, b"\x21\x03"))
        self.data.extend(self.tim.to_bytes())
        self.data.extend(self.data[-4:])
        mark("animations")
        self.animation_start = len(self.data)
        directory_size = field.BUILTIN_ENTRY_COUNT * field.BUILTIN_ENTRY.size
        raw = bytearray(directory_size + 16)
        struct.pack_into("<4I", raw, 0, 0, len(raw), len(raw), 0)
        field.BUILTIN_ENTRY.pack_into(raw, 16, directory_size, directory_size + 2, directory_size + 4, 12, 1, bytes(7))
        raw.extend(bytes(field.BUILTIN_ENTRY_COUNT - field.FIRST_ANIMATION))
        raw.extend(bytes(field.ANIMATION_DEFINITION.size))
        raw.extend(raw[-4:])
        self.data.extend(raw)
        add_table(field.SYMBOL_NAMES["hud_colors"])
        delayed = {
            field.SYMBOL_NAMES[key] for key in ("pixels", "hud_colors", "windows", "levels", "drop_handlers")
        } | {"g_field_target_filters"}
        for spec in field.TABLES:
            if spec.symbol not in delayed:
                add_table(spec.symbol)
        transition = TimImage(2, None, TimBlock(0, 0, 32, 64, struct.pack("<H", 0x7C00) * (32 * 64)))
        self.symbols["g_field_transition_tiles"] = ADDRESS + len(self.data) + 20
        self.symbols["g_field_transition_tiles_alt"] = ADDRESS + len(self.data) + 20 + 32 * 32 * 2
        self.data.extend(transition.to_bytes())
        self.data.extend(self.data[-4:])
        add_table("g_field_target_filters")
        self.ui_start = len(self.data)
        self.symbols[field.SYMBOL_NAMES["title_choice"]] = ADDRESS + len(self.data) + field.TITLE_CHOICE_INDEX * 2
        text = b"Load?" if version == "us" else b"\x41\x19\x00"
        self.data.extend(make_table([text] * 37))
        pad()
        mark("abilities")
        self.data.extend(bytes([1, 2, 3, 4, 5]) * field.ABILITY_RULE_COUNT)
        pad()
        mark("techniques")
        self.data.extend(bytes(range(11)) * field.TECHNIQUE_RULE_COUNT)
        pad()
        self.data.extend(b"SAVE-NAME\x00\x00\x00")
        mark("save_codes")
        self.data.extend(b"BASLUS-00000" * 11)
        add_text("technique_names")
        add_text("command_names")
        self.golem_header = len(self.data)
        self.data.extend(struct.pack("<5I", 4, 20, 532, 1044, 1556))
        mark("golem_palettes")
        self.data.extend(PALETTE * 32)
        mark("golem_portrait_palettes")
        self.data.extend(PALETTE * 32)
        self.data.extend(self.data[-4:])
        self.data.extend(struct.pack("<3I", 2, 12, 44))
        mark("portrait_palettes")
        self.data.extend(PALETTE * 2)
        self.data.extend(self.data[-4:])
        add_text("item_names")
        mark("panel_quads")
        self.data.extend(struct.pack("<HHBBHHH", 32, 96, 0, 1, 32, 16, 0x100) * 12)
        self.data.extend(struct.pack("<2I", 1, 8))
        mark("menu_image")
        self.data.extend(PALETTE * field.MENU_PALETTE_COUNT)
        self.data.extend(b"\x21" * (field.MENU_WIDTH * field.MENU_HEIGHT // 2))
        self.data.extend(self.data[-4:])
        add_text("weekdays")
        add_table(field.SYMBOL_NAMES["windows"])
        mark("actions")
        self.action_start = len(self.data)
        self.data.extend(struct.pack("<4I", 16, 24, 32, 40))
        self.data.extend(struct.pack("<II", 0x03640291, 0x76543210) * 4)
        add_table(field.SYMBOL_NAMES["levels"])
        add_text("item_name_table")
        add_table(field.SYMBOL_NAMES["drop_handlers"])
        mark("golem_shapes")
        for _ in range(field.GOLEM_SHAPE_COUNT):
            self.data.extend(struct.pack("<4B2h", 2, 0, 3, 3, -1, 1))
            self.data.extend(struct.pack("<bbh", -1, 2, 99) * 20)
        mark("variables")
        self.data.extend(bytes(64))

    def write(self, root, skip_symbol=None):
        config, assets = root / "config", root / "assets"
        (config / "symbols").mkdir(parents=True)
        (config / "overlays").mkdir()
        assets.mkdir()
        (assets / "field_data.databin.bin").write_bytes(self.data)
        segment = {"start": 1, "vram": ADDRESS, "subsegments": [[1, "databin", "field_data"]]}
        (config / field.OVERLAY_CONFIG).write_text(yaml.safe_dump({"segments": [segment, [len(self.data) + 1]]}))
        (config / field.SYMBOL_FILES[0]).write_text("test_handler = 0x80012340;\n")
        (config / field.SYMBOL_FILES[1]).write_text("\n".join(
            f"{name} = 0x{address:08X};" for name, address in self.symbols.items() if name != skip_symbol
        ))
        if self.version == "jp":
            chart = FakeCload("jp").write(root / "cload")
            source = chart.overlay_config.read_text().replace("blob", "chart")
            (config / field.cload.OVERLAY_CONFIG).write_text(source)
            shutil.copyfile(chart.symbol_file, config / field.cload.SYMBOL_FILE)
            shutil.copyfile(chart.assets / "blob.databin.bin", assets / "chart.databin.bin")
        return field.Inputs(self.version, config, assets)


def png_pixels(path):
    data = path.read_bytes()
    width, height = struct.unpack_from(">II", data, 16)
    position = 8
    packed = bytearray()
    while position < len(data):
        size = struct.unpack_from(">I", data, position)[0]
        kind = data[position + 4 : position + 8]
        payload = data[position + 8 : position + 8 + size]
        checksum = struct.unpack_from(">I", data, position + 8 + size)[0]
        assert checksum == zlib.crc32(kind + payload) & 0xFFFFFFFF
        if kind == b"IDAT":
            packed.extend(payload)
        position += size + 12
    decoded = zlib.decompress(packed)
    assert len(decoded) == height * (width * 4 + 1)
    return width, height, decoded


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

    def test_exports_images_tables_resources_and_full_byte_map(self):
        field.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "images/common/image.tim").read_bytes(), self.overlay.tim.to_bytes())
        width, height, pixels = png_pixels(self.output / "images/common/palette_00.png")
        self.assertEqual((width, height), (4, 1))
        self.assertEqual(pixels[1:], bytes([255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 0, 0]))
        self.assertEqual(png_pixels(self.output / "images/transition/image.png")[:2], (32, 64))
        self.assertEqual(png_pixels(self.output / "images/menu_frame/palette_01.png")[:2], (64, 32))
        self.assertEqual(png_pixels(self.output / "palettes/golem_portrait_palettes.png")[:2], (128, 256))
        self.assertEqual(self.load("text/ui.yaml")["entries"][0]["text"], "Load?")
        self.assertEqual(self.load("text/known_save_codes.yaml")["codes"][0], "BASLUS-00000")
        rule = self.load("tables/techniques_unlock_rules.yaml")["rules"][0]
        self.assertEqual(rule["weapon_proficiency"], 10)
        self.assertEqual(rule["prerequisites"][0], {"ability": 0, "proficiency": 1})
        pointer = self.load("tables/field_target_filters.yaml")["entries"][0]
        self.assertEqual(pointer["symbols"], ["test_handler"])
        action = self.load("actions/default.yaml")["tables"][0]["entries"][0]
        self.assertEqual((action["kind"], action["power"], action["handler"]), (1, 100, 3))
        shape = self.load("tables/golem_shapes.yaml")["shapes"][0]
        self.assertEqual(shape["origin_x"], -1)
        self.assertEqual(shape["rotations"][0]["parts"][0], {"x": -1, "y": 2, "glyph_id": 99})
        animation = self.load("animations/builtin.yaml")
        self.assertEqual(animation["entries"][0]["duration"], 12)
        self.assertEqual(len(animation["definitions"]), 1)
        cursor = 0
        for entry in self.load("byte-map.yaml")["ranges"]:
            self.assertEqual(int(entry["offset"], 16), cursor)
            cursor += int(entry["size"], 16)
        self.assertEqual(cursor, len(self.overlay.data))
        address = ADDRESS + self.overlay.unknown_start
        self.assertEqual((self.output / f"unknown/{address:08X}.bin").read_bytes(), b"test")

    def test_japanese_text_uses_and_records_the_external_chart(self):
        field.extract(FakeOverlay("jp").write(self.root), self.output)
        entry = self.load("text/ui.yaml")["entries"][0]
        self.assertEqual(entry["text"], "\uff21\uff21")
        self.assertEqual(entry["bytes"], "41 19 00")
        self.assertEqual(self.load("byte-map.yaml")["text_decoder"]["overlay"], "CLOAD")

    def test_missing_japanese_chart_writes_nothing(self):
        inputs = FakeOverlay("jp").write(self.root)
        (inputs.assets / "chart.databin.bin").unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            field.extract(inputs, self.output)
        self.assert_no_output()

    def test_truncated_data_is_rejected(self):
        inputs = self.overlay.write(self.root)
        (inputs.assets / "field_data.databin.bin").write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            field.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_symbol_names_its_source(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_field_resource_blob")
        with self.assertRaisesRegex(ValueError, "g_field_resource_blob.*SYMBOL_NAMES"):
            field.extract(inputs, self.output)
        self.assert_no_output()

    def test_tim_cannot_run_into_animation_data(self):
        start = self.overlay.symbols[field.SYMBOL_NAMES["texture"]] - ADDRESS
        struct.pack_into("<I", self.overlay.data, start + 8, 0x7FFFFFFF)
        with self.assertRaisesRegex(ValueError, "past file size"):
            field.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_animation_definitions_cannot_run_into_tables(self):
        start = self.overlay.animation_start
        table = struct.unpack_from("<I", self.overlay.data, start + 8)[0]
        self.overlay.data[start + table] = 255
        with self.assertRaisesRegex(ValueError, "animation definitions run past"):
            field.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_action_offsets_cannot_run_backwards(self):
        struct.pack_into("<I", self.overlay.data, self.overlay.action_start + 4, 8)
        with self.assertRaisesRegex(ValueError, "invalid action bank offsets"):
            field.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_palette_offsets_are_checked(self):
        struct.pack_into("<I", self.overlay.data, self.overlay.golem_header + 4, 24)
        with self.assertRaisesRegex(ValueError, "invalid golem offsets"):
            field.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_text_cannot_run_into_unlock_rules(self):
        struct.pack_into("<H", self.overlay.data, self.overlay.ui_start + 2, 0xFFFF)
        with self.assertRaisesRegex(ValueError, "invalid offset"):
            field.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_nonzero_variables_are_preserved(self):
        self.overlay.data[-1] = 0x5A
        field.extract(self.overlay.write(self.root), self.output)
        self.assertEqual((self.output / "unknown/variables.bin").read_bytes(), bytes(63) + b"Z")

    def test_failed_write_removes_staging(self):
        with patch.object(field, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                field.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_existing_output_is_preserved(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            field.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
