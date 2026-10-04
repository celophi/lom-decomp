"""Check FIELD's extractor against the regional configs and source layouts."""

import re
import struct
import unittest
from pathlib import Path

from tools.overlays import cload, field, splat_config
from tools.overlays.tests.test_addhero_sources import c_define

REPO_ROOT = Path(__file__).resolve().parents[3]


class SourceTest(unittest.TestCase):
    def test_both_versions_have_one_data_blob_and_all_required_symbols(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = field.FieldSymbols.load(config)
                files = splat_config.data_files(config / field.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
                self.assertEqual(len(files), 1)
                source = files[0]
                self.assertEqual((source.name, source.kind), ("field_data", "databin"))
                self.assertEqual(source.start, names.pixels)
                self.assertTrue(all(source.contains(getattr(names, key)) for key in field.SYMBOL_NAMES))
                for spec in field.TABLES:
                    address = names.named[spec.symbol]
                    size = struct.calcsize("<" + spec.format) * spec.count
                    self.assertTrue(source.contains(address), spec.symbol)
                    self.assertLessEqual(address + size, names.variables, spec.symbol)
                self.assertEqual(names.golem_portrait_palettes - names.golem_palettes, 1024)
                self.assertEqual(names.windows - names.weekdays, 76 if version == "us" else 68)
                # Japanese text must have a chart from the same version.
                if version == "jp":
                    chart = cload.CloadSymbols.load(config / cload.SYMBOL_FILE)
                    self.assertLess(chart.chart, chart.decimal_glyphs)

    def test_counts_follow_the_c_definitions(self):
        specs = {spec.symbol: spec for spec in field.TABLES}
        checks = (
            (field.ABILITY_RULE_COUNT, "src/overlays/field/internal/field_ability_progression.h", "FIELD_ABILITY_UNLOCK_RULE_COUNT"),
            (field.TECHNIQUE_RULE_COUNT, "src/overlays/field/ui/field_dialog_screens.c", "FIELD_TECHNIQUE_UNLOCK_RULE_COUNT"),
            (field.GOLEM_SHAPE_COUNT, "include/common/golem_shape.h", "GOLEM_SHAPE_COUNT"),
            (field.TITLE_CHOICE_INDEX, "src/overlays/field/ui/field_choice_labels.c", "FIELD_TITLE_CHOICE_FIRST_TEXT"),
            (field.MENU_HEIGHT, "src/overlays/field/ui/field_menu_windows.c", "MENU_FRAME_IMAGE_HEIGHT"),
            (field.MENU_PALETTE_COUNT, "src/overlays/field/ui/field_menu_windows.c", "MENU_FRAME_STYLES"),
            (field.TRANSITION_SIZE, "src/overlays/field/scene/field_scene_transition.c", "FIELD_TRANSITION_TILE_SIZE"),
            (specs["g_field_party_palettes"].count, "src/overlays/field/scene/field_scene_transition.c", "FIELD_PARTY_PALETTE_COUNT"),
            (specs["g_field_command_patterns"].count, "src/overlays/field/actors/field_command_history.c", "FIELD_COMMAND_PATTERN_COUNT"),
            (specs["g_equipment_combination_quantity_scale"].count, "src/overlays/field/records/field_equipment_combination_rules.c", "EQUIPMENT_COMBINATION_RULE_COUNT"),
        )
        for value, source, name in checks:
            with self.subTest(name=name):
                self.assertEqual(value, c_define(REPO_ROOT / source, name))
        width_words = c_define(REPO_ROOT / "src/overlays/field/ui/field_menu_windows.c", "MENU_FRAME_IMAGE_WIDTH")
        self.assertEqual(field.MENU_WIDTH, width_words * 4)

    def test_table_layouts_have_unique_symbols_and_consistent_fields(self):
        seen = set()
        for spec in field.TABLES:
            self.assertNotIn(spec.symbol, seen)
            seen.add(spec.symbol)
            layout = struct.Struct("<" + spec.format)
            fields = layout.unpack(bytes(layout.size))
            if spec.fields:
                self.assertEqual(len(spec.fields), len(fields), spec.symbol)
            if spec.resolve_symbols:
                self.assertEqual(spec.format, "I")

    def test_builtin_record_sizes_follow_source_fields(self):
        source = (REPO_ROOT / "src/overlays/field/actors/field_actor_slot_resources.c").read_text()
        entry = re.search(r"typedef struct\s*\{([^}]+)\} FieldBuiltinEntry;", source).group(1)
        self.assertEqual(len(re.findall(r"\bu16\s+\w+;", entry)), 4)
        self.assertIn("u8 unk9[7]", entry)
        self.assertEqual(field.BUILTIN_ENTRY.size, 16)
        self.assertEqual(field.ANIMATION_DEFINITION.size, 28)
        self.assertEqual(field.ACTION_DESCRIPTOR.size, 8)
        self.assertEqual(field.GOLEM_SHAPE_BYTES, 8 + 4 * 5 * 4)


if __name__ == "__main__":
    unittest.main()
