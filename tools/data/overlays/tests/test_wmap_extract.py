"""Run WMAP's extractor on a small synthetic overlay, without disc data."""

import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.data.overlays import wmap
from tools.data.overlays.wmap_tables import TABLES

ADDRESS = 0x800C0000
STEP_A = 0x80010000
STEP_B = 0x80010010
DECLARED = {"test_steps": "effects/wmap_test_effect", "test_missing_steps": "effects/wmap_test_effect"}


def sound_buffer(sequences: list[tuple[int, int]]) -> bytes:
    """An AKAO sound-effect list with one entry per (first, second) sequence offset pair."""
    raw = bytearray(b"AKAO" + struct.pack("<iiI", len(sequences), 7, 0))
    raw += struct.pack(f"<{len(sequences)}i", *[index * 8 for index in range(len(sequences))])
    raw += bytes(wmap.SOUND_DATA - len(raw))
    for first, second in sequences:
        raw += struct.pack("<2H", first, second) + b"\xa0\xa1\xa2\xa3"
    return bytes(raw)


class FakeOverlay:
    """Every table WMAP reads, three sound buffers, three input scripts and one step table."""

    def __init__(self):
        self.symbols = {"wmap_test_step_a": STEP_A, "wmap_test_step_b": STEP_B}
        self.sizes = {}
        self.data = bytearray()
        specs = list(TABLES)

        def here():
            return ADDRESS + len(self.data)

        def add_table(spec):
            self.symbols[spec.symbol] = here()
            self.data.extend(bytes(struct.calcsize("<" + spec.format) * spec.count))

        for spec in specs[:8]:
            add_table(spec)
        self.sounds = [sound_buffer([(0, 2)]), sound_buffer([(4, 0xFFFF), (0xFFFF, 0)]), sound_buffer([(0, 0)])]
        pointers = []
        for raw in self.sounds:
            pointers.append(here())
            self.data.extend(raw)
        self.symbols[wmap.SYMBOL_NAMES["sounds"]] = here()
        pointers += [pointers[0]] * (wmap.SOUND_COUNT - len(pointers))
        self.data.extend(struct.pack(f"<{wmap.SOUND_COUNT}I", *pointers))
        for spec in specs[8:]:
            add_table(spec)
            if spec.symbol == "g_wmap_carousel_rotation":
                self.unknown = here()
                self.data.extend(b"test")
        scripts = [(-2, 2, 0, -2, 1, -4032, 30, 0x80, 0, 0x8F0, -1), (-2, 3, -1), (-1,)]
        addresses = []
        for words in scripts:
            addresses.append(here())
            self.data.extend(struct.pack(f"<{len(words)}h", *words))
        self.data.extend(bytes(-len(self.data) % 4))
        self.symbols[wmap.SYMBOL_NAMES["input_scripts"]] = here()
        self.data.extend(struct.pack("<3I", *addresses))
        self.symbols["test_steps"] = here()
        self.steps_offset = len(self.data)
        self.symbols[wmap.SYMBOL_NAMES["variables"]] = here() + 12
        self.data.extend(struct.pack("<3I", STEP_A, 0, self.symbols[wmap.SYMBOL_NAMES["variables"]]))
        self.data.extend(bytes(32))

    def offset(self, symbol):
        return self.symbols[symbol] - ADDRESS

    def write(self, root, skip_symbol=None):
        config, assets = root / "config", root / "assets"
        (config / "symbols").mkdir(parents=True, exist_ok=True)
        (config / "overlays").mkdir(exist_ok=True)
        assets.mkdir(exist_ok=True)
        (assets / "wmap_data.databin.bin").write_bytes(self.data)
        segment = {"start": 1, "vram": ADDRESS, "subsegments": [[1, "databin", "wmap_data"]]}
        (config / wmap.OVERLAY_CONFIG).write_text(yaml.safe_dump({"segments": [segment, [1 + len(self.data)]]}))
        lines = [f"{name} = 0x{address:08X};" + (f" // size:0x{self.sizes[name]:X}" if name in self.sizes else "")
                 for name, address in self.symbols.items() if name != skip_symbol]
        (config / wmap.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return wmap.Inputs("us", config, assets)


class ExtractTest(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        self.output = self.root / "out"
        self.overlay = FakeOverlay()

    def extract(self):
        wmap.extract(self.overlay.write(self.root), self.output, DECLARED)

    def load(self, name):
        return yaml.safe_load((self.output / name).read_text(encoding="utf-8"))

    def assert_no_output(self):
        self.assertFalse(self.output.exists())
        self.assertEqual(list(self.root.glob(".out-*")), [])

    def test_byte_map_covers_every_byte_and_keeps_unknown_data(self):
        self.extract()
        cursor = 0
        parts = self.load("byte-map.yaml")["ranges"]
        for part in parts:
            self.assertEqual(int(part["offset"], 16), cursor)
            cursor += int(part["size"], 16)
            if "file" in part:
                self.assertTrue((self.output / part["file"]).is_file(), part["file"])
        self.assertEqual(cursor, len(self.overlay.data))
        self.assertEqual(parts[-1]["name"], "runtime state")
        self.assertNotIn("file", parts[-1])
        self.assertEqual((self.output / f"unknown/{self.overlay.unknown:08X}.bin").read_bytes(), b"test")
        self.assertIn("padding", [part["name"] for part in parts])

    def test_tables_decode_their_records(self):
        struct.pack_into("<I4B2h2BH2h", self.overlay.data, 0, 0x04000000, 128, 64, 32, 0x66, -5, 60, 16, 8, 0x7E2C, 30, 12)
        attributes = self.overlay.offset("g_wmap_land_attributes") + 12
        struct.pack_into("<8BI", self.overlay.data, attributes, 1, 2, 3, 4, 5, 6, 7, 8, 0x80000001)
        struct.pack_into("<I", self.overlay.data, self.overlay.offset("g_wmap_controller_ports"), STEP_B)
        self.extract()
        sprite = self.load("tables/game_continue_prompt.yaml")
        self.assertEqual(sprite["entries"][0], {"tag": 0x04000000, "r0": 128, "g0": 64, "b0": 32, "code": 0x66,
                                                "x0": -5, "y0": 60, "u0": 16, "v0": 8, "clut": 0x7E2C, "w": 30, "h": 12})
        self.assertEqual(sprite["symbol"], "g_wmap_game_continue_prompt")
        land = self.load("tables/land_attributes.yaml")["entries"][1]
        self.assertEqual([land[f"spirit_{index}"] for index in range(8)], [1, 2, 3, 4, 5, 6, 7, 8])
        self.assertEqual(land["flags"], 0x80000001)
        pointer = self.load("tables/controller_ports.yaml")["entries"][0]
        self.assertEqual(pointer, {"value": "0x80010010", "symbols": ["wmap_test_step_b"]})
        self.assertEqual(len(self.load("tables/artifact_pickup_frames.yaml")["entries"]), 740)

    def test_sounds_keep_each_buffer_and_decode_its_entries(self):
        self.extract()
        for index, raw in enumerate(self.overlay.sounds):
            self.assertEqual((self.output / f"sounds/sound_{index + 1:02d}.akao").read_bytes(), raw)
        sounds = self.load("sounds/sounds.yaml")["entries"]
        self.assertEqual(len(sounds), wmap.SOUND_COUNT)
        self.assertEqual(sounds[0]["entries"], [{"offset": "0x20", "sequences": ["0x24", "0x26"]}])
        self.assertEqual(sounds[0]["bank_key"], 7)
        self.assertEqual(sounds[1]["entries"], [{"offset": "0x20", "sequences": ["0x28", None]},
                                                {"offset": "0x28", "sequences": [None, "0x2C"]}])
        self.assertEqual(sounds[5]["same_as"], 1)
        self.assertFalse((self.output / "sounds/sound_06.akao").exists())

    def test_input_scripts_follow_the_script_codes(self):
        self.extract()
        scripts = self.load("scripts/input_scripts.yaml")["scripts"]
        self.assertEqual(scripts[0]["steps"], [
            {"code": "WMAP_SCRIPT_COMMAND", "command": "WMAP_SCRIPT_VISIBILITY", "argument": 0},
            {"code": "WMAP_SCRIPT_COMMAND", "command": "WMAP_SCRIPT_BUTTON_MASK", "argument": "0xF040"},
            {"hold_frames": 30, "buttons": "0x0080"},
            {"wait_buttons": "0x08F0"},
            {"code": "WMAP_SCRIPT_END"},
        ])
        self.assertEqual(scripts[1]["steps"][0]["command"], "WMAP_SCRIPT_WAIT_ARTIFACT")
        self.assertEqual(scripts[2]["bytes"], "ff ff")

    def test_step_tables_name_functions_and_data_targets(self):
        self.extract()
        handlers = self.load("handlers/wmap_test_effect.yaml")
        self.assertEqual(handlers["source"], "src/overlays/wmap/effects/wmap_test_effect.c")
        self.assertEqual(handlers["tables"], [{
            "symbol": "test_steps", "address": f"0x{self.overlay.symbols['test_steps']:08X}", "count": 3,
            "steps": ["wmap_test_step_a", None, {"data": "g_wmap_game_displayed_score"}],
        }])

    def test_step_target_inside_a_sized_symbol_is_named_by_offset(self):
        self.overlay.sizes["g_wmap_game_displayed_score"] = 0x10
        struct.pack_into("<I", self.overlay.data, self.overlay.steps_offset + 4, self.overlay.symbols["g_wmap_game_displayed_score"] + 8)
        self.extract()
        steps = self.load("handlers/wmap_test_effect.yaml")["tables"][0]["steps"]
        self.assertEqual(steps[1], {"data": "g_wmap_game_displayed_score+0x8"})

    def test_unknown_step_target_writes_nothing(self):
        struct.pack_into("<I", self.overlay.data, self.overlay.steps_offset + 4, 0x80012345)
        with self.assertRaisesRegex(ValueError, r"test_steps\[1\].*not a known symbol"):
            self.extract()
        self.assert_no_output()

    def test_sound_without_akao_header_writes_nothing(self):
        self.overlay.data[self.overlay.offset(wmap.SYMBOL_NAMES["sounds"]) - len(self.overlay.sounds[2])] ^= 1
        with self.assertRaisesRegex(ValueError, "sound effect 3 has no AKAO header"):
            self.extract()
        self.assert_no_output()

    def test_sound_pointer_outside_the_sound_run_writes_nothing(self):
        struct.pack_into("<I", self.overlay.data, self.overlay.offset(wmap.SYMBOL_NAMES["sounds"]), ADDRESS - 4)
        with self.assertRaisesRegex(ValueError, "sound effect pointers"):
            self.extract()
        self.assert_no_output()

    def test_sound_entry_outside_its_buffer_writes_nothing(self):
        start = self.overlay.offset(wmap.SYMBOL_NAMES["sounds"]) - sum(len(raw) for raw in self.overlay.sounds)
        struct.pack_into("<i", self.overlay.data, start + wmap.SOUND_OFFSETS, 0x100)
        with self.assertRaisesRegex(ValueError, "sound effect 1 has an entry outside"):
            self.extract()
        self.assert_no_output()

    def test_script_without_end_code_writes_nothing(self):
        table = self.overlay.offset(wmap.SYMBOL_NAMES["input_scripts"])
        struct.pack_into("<h", self.overlay.data, table - 4, 5)
        with self.assertRaisesRegex(ValueError, "no end code"):
            self.extract()
        self.assert_no_output()

    def test_missing_symbol_reports_what_to_update(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_wmap_land_attributes")
        with self.assertRaisesRegex(ValueError, "g_wmap_land_attributes.*wmap_tables.py"):
            wmap.extract(inputs, self.output, DECLARED)
        self.assert_no_output()

    def test_wrong_resource_order_is_rejected(self):
        self.overlay.symbols[wmap.SYMBOL_NAMES["input_scripts"]] = self.overlay.symbols[wmap.SYMBOL_NAMES["sounds"]] - 4
        with self.assertRaisesRegex(ValueError, "out of resource order"):
            self.extract()
        self.assert_no_output()

    def test_missing_or_truncated_blob_writes_nothing(self):
        inputs = self.overlay.write(self.root)
        path = inputs.assets / "wmap_data.databin.bin"
        path.write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            wmap.extract(inputs, self.output, DECLARED)
        path.unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            wmap.extract(inputs, self.output, DECLARED)
        self.assert_no_output()

    def test_nonzero_runtime_state_is_preserved(self):
        self.overlay.data[-1] = 0x5A
        self.extract()
        self.assertEqual((self.output / "unknown/variables.bin").read_bytes(), bytes(31) + b"Z")

    def test_failed_write_removes_staging(self):
        with patch.object(wmap, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                self.extract()
        self.assert_no_output()

    def test_existing_output_is_preserved(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            self.extract()
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
