"""Check WMAP's extractor against the regional configs and the C layouts."""

import re
import struct
import unittest
from pathlib import Path

from tools.overlays import splat_config, wmap
from tools.overlays.tests.test_addhero_sources import c_define

REPO_ROOT = Path(__file__).resolve().parents[3]
SOURCES = REPO_ROOT / "src/overlays/wmap"
DATA_FILES = [
    ("wmap_land_preview_rodata_alignment", "rodatabin"),
    ("wmap_model_render_rodata_alignment", "rodatabin"),
    ("wmap_data", "databin"),
]


def struct_body(path: Path, name: str) -> str:
    match = re.search(r"typedef (?:struct|union)\s*\{((?:[^{}]|\{[^{}]*\})*)\}\s*" + name + r";", path.read_text())
    if match is None:
        raise AssertionError(f"{path} has no {name}")
    return match.group(1)


class SourceTest(unittest.TestCase):
    def load(self, version):
        config = REPO_ROOT / "config" / version
        names = wmap.WmapSymbols.load(config / wmap.SYMBOL_FILE)
        files = splat_config.data_files(config / wmap.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
        return names, files

    def test_both_versions_have_one_data_blob_and_ordered_anchors(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                names, files = self.load(version)
                self.assertEqual([(file.name, file.kind) for file in files], DATA_FILES)
                source = files[-1]
                self.assertEqual(source.start, names.first)
                addresses = [getattr(names, key) for key in wmap.SYMBOL_NAMES]
                self.assertEqual(addresses, sorted(set(addresses)))
                self.assertTrue(all(source.contains(address) for address in addresses))

    def test_tables_fit_before_the_next_resource(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                names, _ = self.load(version)
                ranges = [(names.named[spec.symbol], struct.calcsize("<" + spec.format) * spec.count, spec.symbol)
                          for spec in wmap.TABLES]
                ranges += [(names.sounds, wmap.SOUND_COUNT * 4, "sounds"),
                           (names.input_scripts, wmap.INPUT_SCRIPT_COUNT * 4, "input scripts")]
                ranges.sort()
                for (address, size, symbol), (following, _, _) in zip(ranges, ranges[1:] + [(names.variables, 0, "")]):
                    self.assertLessEqual(address + size, following, symbol)

    def test_pointer_tables_fill_their_stored_runs(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                names, _ = self.load(version)
                self.assertEqual(names.named["g_wmap_information_glyphs"] - names.sounds, wmap.SOUND_COUNT * 4)
                self.assertEqual(names.named["D_800D0550"] - names.input_scripts, wmap.INPUT_SCRIPT_COUNT * 4)

    def test_handler_tables_are_named_in_both_versions(self):
        declared = wmap.handler_declarations()
        self.assertGreater(len(declared), 400)
        self.assertEqual(declared["g_wmap_effect35_steps"], "effects/wmap_special_effect_35")
        self.assertEqual(declared["g_wmap_special_travel_steps"], "travel/wmap_travel_sequences")
        for source in declared.values():
            self.assertTrue((SOURCES / f"{source}.c").is_file(), source)
        for version in ("us", "jp"):
            with self.subTest(version=version):
                names, _ = self.load(version)
                stored = [name for name in declared if names.first <= names.named.get(name, 0) < names.variables]
                self.assertEqual(len(stored), 453)

    def test_counts_follow_the_c_definitions(self):
        specs = {spec.symbol: spec for spec in wmap.TABLES}
        display = REPO_ROOT / "src/overlays/wmap/internal/wmap_map_display.h"
        checks = (
            (specs["g_wmap_artifact_positions"].count, REPO_ROOT / "src/overlays/wmap/internal/wmap_land_transition.h", "WMAP_ARTIFACT_SLOTS"),
            (specs["g_wmap_land_attributes"].count, SOURCES / "map/wmap_map_display.c", "WMAP_LAND_COUNT"),
            (specs["g_wmap_menu_triangles"].count, SOURCES / "wmap_main.c", "WMAP_MENU_TRIANGLE_COUNT"),
        )
        for value, source, name in checks:
            with self.subTest(name=name):
                self.assertEqual(value, c_define(source, name))
        scales = c_define(display, "WMAP_QUAD_SCALE_VARIANTS") * c_define(display, "WMAP_QUAD_SCALE_STEPS")
        self.assertEqual(specs["g_wmap_land_quad_scales"].count, scales)
        self.assertIn("WmapQuadTemplate g_wmap_cell_effect_quads[16];", (SOURCES / "map/wmap_map_display.c").read_text())
        self.assertEqual(specs["g_wmap_cell_effect_quads"].count, 16)

    def test_record_sizes_follow_source_fields(self):
        specs = {spec.symbol: spec for spec in wmap.TABLES}
        image = struct_body(REPO_ROOT / "src/overlays/wmap/internal/wmap_land_transition.h", "WmapArtifactImage")
        self.assertEqual(len(re.findall(r"\bu8\s+\w+;", image)), 4)
        self.assertEqual(len(re.findall(r"\bu16\s+\w+;", image)), 6)
        self.assertEqual(len(re.findall(r"\bs16\s+\w+;", image)), 4)
        self.assertEqual(struct.calcsize("<" + specs["g_wmap_artifact_images"].format), 24)
        frame = struct_body(SOURCES / "map/wmap_land_preview.c", "WmapArtifactTransferFrame")
        self.assertEqual(len(re.findall(r"\bs16\s+\w+;", frame)), 6)
        self.assertEqual(struct.calcsize("<" + specs["g_wmap_artifact_pickup_frames"].format), 12)
        glyph = struct_body(SOURCES / "map/wmap_map_display.c", "WmapGlyph")
        self.assertIn("u8 u, v, width, height;", glyph)
        self.assertEqual(struct.calcsize("<" + specs["g_wmap_information_glyphs"].format), 8)
        texture = struct_body(SOURCES / "render/wmap_sprite_render.c", "WmapSpriteTexture")
        self.assertIn("u16 clut[8];", texture)
        self.assertEqual(struct.calcsize("<" + specs["g_wmap_sprite_textures"].format), 28)
        attributes = struct_body(SOURCES / "map/wmap_land_layout.c", "WmapLandAttributes")
        self.assertIn("u8 bytes[12];", attributes)
        self.assertEqual(struct.calcsize("<" + specs["g_wmap_land_attributes"].format), 12)
        label = struct_body(SOURCES / "render/wmap_map_labels.c", "WmapLabelChar")
        self.assertEqual(len(re.findall(r"\bu8\s+\w+;", label)), 4)
        self.assertEqual(struct.calcsize("<" + specs["D_800D040C"].format), 6)
        for symbol, size in (("g_wmap_game_continue_prompt", 20), ("g_wmap_backdrop_front_quads", 40),
                             ("D_800D06BC", 36), ("g_wmap_menu_triangles", 28), ("g_wmap_transition_mesh_triangles_0", 28)):
            self.assertEqual(struct.calcsize("<" + specs[symbol].format), size, symbol)

    def test_table_layouts_have_unique_symbols_and_consistent_fields(self):
        seen = set()
        for spec in wmap.TABLES:
            self.assertNotIn(spec.symbol, seen)
            seen.add(spec.symbol)
            layout = struct.Struct("<" + spec.format)
            if spec.fields:
                self.assertEqual(len(spec.fields), len(layout.unpack(bytes(layout.size))), spec.symbol)
            if spec.resolve_symbols:
                self.assertEqual(spec.format, "I")

    def test_script_codes_come_from_the_owning_enum(self):
        names = wmap.script_names()
        self.assertEqual(names[-1], "WMAP_SCRIPT_END")
        self.assertEqual(names[-2], "WMAP_SCRIPT_COMMAND")
        self.assertEqual(names[0], "WMAP_SCRIPT_IMAGE")
        self.assertTrue(set(wmap.SCRIPT_ARGUMENT_COMMANDS) <= set(names.values()))


if __name__ == "__main__":
    unittest.main()
