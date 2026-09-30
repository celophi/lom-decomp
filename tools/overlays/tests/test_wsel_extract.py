"""Exercise WSEL's export using invented data, without game files."""

import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import yaml

from tools.assets.psx_tim import TimBlock, TimImage
from tools.overlays import wsel
from tools.overlays.tests.test_field_extract import png_pixels

ADDRESS = 0x8004FC90
EIGHT_BIT_LAYERS = (0, 1, 4, 5)


def pixel(rows, width, x, y):
    start = y * (width * 4 + 1) + 1 + x * 4
    return tuple(rows[start:start + 4])


class FakeOverlay:
    """Tiny TIMs and made-up grid, layer and pose tables in the real order."""

    def __init__(self):
        self.symbols = {}
        self.data = bytearray()
        self.tims = []

        def mark(key):
            self.symbols[wsel.SYMBOL_NAMES[key]] = ADDRESS + len(self.data)

        for index, key in enumerate(wsel.TIM_KEYS):
            mark(key)
            if index in EIGHT_BIT_LAYERS:
                palette = struct.pack("<256H", 0, 0x001F, 0x03E0, *([0x7C00] * 253))
                # 16 x 8 pixels; the right-hand 8 x 8 cell holds color 1, then color 2 in its last row.
                pixels = bytes([0] * 8 + [1] * 8) * 7 + bytes([0] * 8 + [2] * 8)
                tim = TimImage(9, TimBlock(index, 480, 256, 1, palette), TimBlock(64, 0, 8, 8, pixels))
            else:
                palette = struct.pack("<16H", 0, 0x7FFF, 0x001F, *([0x03E0] * 13))
                tim = TimImage(8, TimBlock(0, 490 + index, 16, 1, palette), TimBlock(0, 0, 1, 2, bytes.fromhex("10 32 21 00")))
            self.tims.append(tim)
            self.data += tim.to_bytes()
            self.data += self.data[-4:]
        mark("occupied")
        self.occupied = bytes(index % 2 for index in range(361))
        self.data += self.occupied + bytes(3)
        mark("tile_masks")
        self.masks = bytes((index // 36 + index) % 3 == 0 for index in range(361 * 36))
        self.data += self.masks
        mark("sprites")
        for index in range(8):
            mode = 1 if index in EIGHT_BIT_LAYERS else 0
            self.data += struct.pack("<4B10H", mode, index % 4, index & 1, 128, 320 + index * 64, 256,
                                     index, 480 + index, 0, 0, 16, 8, 200 + index, 100)
        mark("poses_default")
        self.poses = struct.pack("<6B", 1, 0, 1, 1, 4, 6) + bytes(6) * 11
        self.data += self.poses
        mark("poses_alternate")
        self.data += struct.pack("<6B", 0, 0, 2, 1, 4, 6) + bytes(6) * 11
        mark("variables")
        self.data += bytes(32)

    def offset(self, key):
        return self.symbols[wsel.SYMBOL_NAMES[key]] - ADDRESS

    def write(self, root, skip_symbol=None):
        config, assets = root / "config", root / "assets"
        (config / "symbols").mkdir(parents=True, exist_ok=True)
        (config / "overlays").mkdir(exist_ok=True)
        assets.mkdir(exist_ok=True)
        (assets / "wsel_data.databin.bin").write_bytes(self.data)
        segment = {"start": 1, "vram": ADDRESS, "subsegments": [[1, "databin", "wsel_data"]]}
        (config / wsel.OVERLAY_CONFIG).write_text(yaml.safe_dump({"segments": [segment, [1 + len(self.data)]]}))
        lines = [f"{name} = 0x{address:08X};" for name, address in self.symbols.items() if name != skip_symbol]
        (config / wsel.SYMBOL_FILE).write_text("\n".join(lines) + "\n")
        return wsel.Inputs("us", config, assets)


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

    def test_images_keep_the_tim_and_list_the_layer_destination(self):
        wsel.extract(self.overlay.write(self.root), self.output)
        for index, key in enumerate(wsel.TIM_KEYS):
            self.assertEqual((self.output / f"images/{key}/image.tim").read_bytes(), self.overlay.tims[index].to_bytes())
        image = self.load("images/cursor/image.yaml")
        self.assertEqual((image["width"], image["height"], image["depth"]), (4, 2, "4bpp"))
        self.assertEqual(image["sprite_layer"], {"index": 2, "name": "WSEL_SPRITE_CURSOR"})
        self.assertEqual(image["uploaded_to"], {"pixels": [448, 256], "palette": [2, 482]})
        self.assertEqual(image["stored_layout"]["clut"]["y"], 492)
        self.assertEqual(image["palettes"], [{"index": 0, "file": "palette_00.png"}])

    def test_palette_previews_decode_both_depths(self):
        wsel.extract(self.overlay.write(self.root), self.output)
        width, height, rows = png_pixels(self.output / "images/cursor/palette_00.png")
        self.assertEqual((width, height), (4, 2))
        self.assertEqual(pixel(rows, width, 0, 0), (0, 0, 0, 0))
        self.assertEqual(pixel(rows, width, 1, 0), (255, 255, 255, 255))
        self.assertEqual(pixel(rows, width, 2, 0), (255, 0, 0, 255))
        self.assertEqual(pixel(rows, width, 0, 1), (255, 255, 255, 255))
        self.assertEqual(pixel(rows, width, 1, 1), (255, 0, 0, 255))
        width, height, rows = png_pixels(self.output / "images/land_map/palette_00.png")
        self.assertEqual((width, height), (16, 8))
        self.assertEqual(pixel(rows, width, 7, 0), (0, 0, 0, 0))
        self.assertEqual(pixel(rows, width, 8, 0), (255, 0, 0, 255))
        self.assertEqual(pixel(rows, width, 15, 7), (0, 255, 0, 255))

    def test_hero_poses_decode_and_crop_their_sheet(self):
        wsel.extract(self.overlay.write(self.root), self.output)
        poses = self.load("tables/hero_poses_default.yaml")
        self.assertEqual(poses["entries"][0], {"index": 0, "u": 1, "v": 0, "width": 1, "height": 1,
                                               "x_offset": 4, "y_offset": 6,
                                               "file": "images/hero_default/poses/00.png"})
        self.assertNotIn("file", poses["entries"][1])
        self.assertEqual(bytes.fromhex(poses["bytes"]), self.overlay.poses)
        width, height, rows = png_pixels(self.output / "images/hero_default/poses/00.png")
        self.assertEqual((width, height), (8, 8))
        self.assertEqual(pixel(rows, width, 0, 0), (255, 0, 0, 255))
        self.assertEqual(pixel(rows, width, 7, 7), (0, 255, 0, 255))
        self.assertEqual(len(list((self.output / "images/hero_default/poses").glob("*.png"))), 1)
        width, height, _ = png_pixels(self.output / "images/hero_alternate/poses/00.png")
        self.assertEqual((width, height), (16, 8))
        self.assertFalse((self.output / "images/hero_shadow/poses").exists())

    def test_grid_tables_decode_rows_and_tile_masks(self):
        wsel.extract(self.overlay.write(self.root), self.output)
        occupied = self.load("tables/cell_occupied.yaml")
        self.assertEqual(len(occupied["rows"]), 19)
        self.assertEqual(occupied["rows"][0], "0101010101010101010")
        self.assertEqual(occupied["rows"][1], "1010101010101010101")
        self.assertEqual(bytes.fromhex(occupied["bytes"]), self.overlay.occupied)
        masks = self.load("tables/cell_tile_masks.yaml")
        self.assertEqual(len(masks["entries"]), 361)
        entry = masks["entries"][20]
        self.assertEqual((entry["column"], entry["row"]), (1, 1))
        expected = ["".join("1" if self.overlay.masks[20 * 36 + row * 6 + column] else "0" for column in range(6))
                    for row in range(6)]
        self.assertEqual(entry["tiles"], expected)

    def test_sprite_layers_take_names_from_wsel_c(self):
        wsel.extract(self.overlay.write(self.root), self.output)
        layers = self.load("tables/sprite_layers.yaml")["entries"]
        self.assertEqual([layer["name"] for layer in layers][4:7],
                         ["WSEL_SPRITE_HERO_DEFAULT", "WSEL_SPRITE_HERO_ALTERNATE", "WSEL_SPRITE_HERO_SHADOW"])
        self.assertEqual(layers[6]["blend"], "subtract")
        self.assertEqual((layers[7]["x"], layers[7]["y"], layers[7]["image"]), (207, 100, "images/prompt/image.yaml"))

    def test_byte_map_covers_every_byte_including_runtime_state(self):
        wsel.extract(self.overlay.write(self.root), self.output)
        cursor = 0
        parts = self.load("byte-map.yaml")["ranges"]
        for part in parts:
            self.assertEqual(int(part["offset"], 16), cursor)
            cursor += int(part["size"], 16)
            if "file" in part:
                self.assertTrue((self.output / part["file"]).is_file())
        self.assertEqual(cursor, len(self.overlay.data))
        self.assertEqual([part["name"] for part in parts if part["name"] == "padding"], ["padding"])
        self.assertEqual(parts[-1]["name"], "screen state")
        self.assertNotIn("file", parts[-1])
        self.assertFalse((self.output / "unknown").exists())

    def test_nonzero_gap_and_runtime_state_are_preserved(self):
        self.overlay.data[self.overlay.offset("tile_masks") - 1] = 0x33
        self.overlay.data[-1] = 0x5A
        wsel.extract(self.overlay.write(self.root), self.output)
        address = ADDRESS + self.overlay.offset("tile_masks") - 3
        self.assertEqual((self.output / f"unknown/{address:08X}.bin").read_bytes(), b"\0\0\x33")
        self.assertEqual((self.output / "unknown/variables.bin").read_bytes(), bytes(31) + b"Z")

    def test_missing_symbol_reports_what_to_update(self):
        inputs = self.overlay.write(self.root, skip_symbol="g_wsel_sprites")
        with self.assertRaisesRegex(ValueError, "g_wsel_sprites.*SYMBOL_NAMES"):
            wsel.extract(inputs, self.output)
        self.assert_no_output()

    def test_missing_or_truncated_blob_writes_nothing(self):
        inputs = self.overlay.write(self.root)
        path = inputs.assets / "wsel_data.databin.bin"
        path.write_bytes(self.overlay.data[:-1])
        with self.assertRaisesRegex(ValueError, "size does not match"):
            wsel.extract(inputs, self.output)
        path.unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            wsel.extract(inputs, self.output)
        self.assert_no_output()

    def test_wrong_resource_order_is_rejected(self):
        self.overlay.symbols[wsel.SYMBOL_NAMES["world_map"]] = ADDRESS
        with self.assertRaisesRegex(ValueError, "out of resource order"):
            wsel.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_incomplete_tables_are_rejected(self):
        for key, message in (("poses_default", "sprite layer records"), ("variables", "alternate hero poses"),
                             ("tile_masks", "cell tile masks")):
            with self.subTest(key=key):
                overlay = FakeOverlay()
                overlay.symbols[wsel.SYMBOL_NAMES[key]] -= 2
                root = self.root / key
                with self.assertRaisesRegex(ValueError, message):
                    wsel.extract(overlay.write(root), root / "out")
                self.assertFalse((root / "out").exists())

    def test_pose_outside_its_sheet_writes_nothing(self):
        self.overlay.data[self.overlay.offset("poses_alternate") + 6 * 3] = 3
        self.overlay.data[self.overlay.offset("poses_alternate") + 6 * 3 + 2] = 1
        self.overlay.data[self.overlay.offset("poses_alternate") + 6 * 3 + 3] = 1
        with self.assertRaisesRegex(ValueError, "hero_alternate pose 3 lies outside"):
            wsel.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_tim_depth_must_match_its_layer(self):
        self.overlay.data[self.overlay.offset("sprites") + 24 * 2] = 1
        with self.assertRaisesRegex(ValueError, "cursor TIM depth differs"):
            wsel.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_tims_trailing_word_must_really_be_a_copy(self):
        self.overlay.data[self.overlay.offset("cursor") - 1] ^= 1
        with self.assertRaisesRegex(ValueError, "does not duplicate"):
            wsel.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_failed_write_removes_staging(self):
        with patch.object(wsel, "write_part", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(OSError, "disk full"):
                wsel.extract(self.overlay.write(self.root), self.output)
        self.assert_no_output()

    def test_existing_output_is_preserved(self):
        self.output.mkdir()
        marker = self.output / "keep.txt"
        marker.write_text("keep me")
        with self.assertRaises(FileExistsError):
            wsel.extract(self.overlay.write(self.root), self.output)
        self.assertEqual(marker.read_text(), "keep me")


if __name__ == "__main__":
    unittest.main()
