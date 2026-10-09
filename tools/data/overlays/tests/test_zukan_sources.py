"""Keep ZUKAN's extractor aligned with the regional maps and the C code."""

import re
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from tools.data.overlays import cload, splat_config, zukan
from tools.data.overlays.tests.test_addhero_sources import c_define

REPO_ROOT = Path(__file__).resolve().parents[4]
DISPLAY_HEADER = REPO_ROOT / "include/main/display.h"


class SourceTest(unittest.TestCase):
    def test_regions_have_one_table_blob_the_entry_code_and_one_data_blob(self):
        lengths = zukan.table_lengths()
        for version in ("us", "jp"):
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = zukan.ZukanSymbols.load(config / zukan.SYMBOL_FILE)
                files = splat_config.data_files(config / zukan.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
                self.assertEqual([(f.name, f.kind) for f in files], [
                    ("zukan_entry_tables", "rodatabin"), ("zukan_run_code", "rodatabin"), ("zukan_data", "databin")])
                tables, code, data = files
                table_addresses = [getattr(names, key) for key in zukan.TABLE_KEYS]
                self.assertEqual(table_addresses, sorted(set(table_addresses)))
                self.assertEqual(tables.start, names.category_ranges)
                self.assertEqual(names.group_ranges - names.category_ranges, lengths["category_ranges"] * 4)
                self.assertEqual(names.entry_values - names.group_ranges, lengths["group_ranges"] * 4)
                self.assertEqual(names.display_order - names.entry_values, lengths["entry_values"] * 2)
                # The display order ends two bytes before zukan_run; those two bytes are padding.
                self.assertEqual(tables.end - names.display_order, lengths["display_order"] * 2 + 2)
                self.assertEqual(code.start, tables.end)
                run = zukan.symbols.load(config / zukan.SYMBOL_FILE)["zukan_run"]
                self.assertEqual(code.start, run)
                data_addresses = [getattr(names, key) for key in zukan.DATA_KEYS]
                self.assertEqual(data_addresses, sorted(set(data_addresses)))
                self.assertEqual(data.start, names.archive)
                self.assertTrue(all(data.contains(address) for address in data_addresses))
                self.assertGreaterEqual(names.variables - names.sprites, zukan.SPRITE_COUNT * zukan.SPRITE_RECORD.size)
                if version == "jp":
                    chart = cload.CloadSymbols.load(config / cload.SYMBOL_FILE)
                    self.assertLess(chart.chart, chart.decimal_glyphs)

    def test_table_lengths_come_from_the_category_structs(self):
        self.assertEqual(zukan.table_lengths(), {
            "category_ranges": 13, "group_ranges": 7, "entry_values": 1018, "display_order": 719})
        self.assertEqual(len(zukan.ARCHIVE_SECTIONS), 4)

    def test_constants_follow_the_c_code(self):
        self.assertEqual(zukan.ENTRY_RESOURCE_BASE, c_define(zukan.SOURCE, "ZUKAN_ENTRY_RESOURCE_BASE"))
        source = zukan.SOURCE.read_text(encoding="ascii")
        record = re.search(r"typedef struct\s*\{([^}]+)\}\s*ZukanUiSpriteRecord;", source).group(1)
        self.assertEqual(re.findall(r"\b(u32|u16)\s+\w+;", record), ["u32", "u32", "u16", "u16"])
        self.assertEqual(zukan.SPRITE_RECORD.size, 12)
        self.assertEqual(zukan.SPRITE_COUNT, c_define(zukan.SOURCE, "ZUKAN_UI_SPRITE_COUNT"))
        self.assertIn("i < ZUKAN_UI_SPRITE_COUNT", source)
        self.assertIn(f"x + {zukan.SPRITE_SCREEN_X_OFFSET}, y", source)
        self.assertIn("(sprite_record->v_clut_and_size >> 14) & 0x1FF, sprite_record->v_clut_and_size >> 23", source)
        self.assertIn("((sprite_record->v_clut_and_size >> 8) & 0x3F) | getClut(0, VRAM_CLUT_Y)", source)
        self.assertEqual(zukan.CLUT_UPLOAD, (0, c_define(DISPLAY_HEADER, "VRAM_CLUT_Y")))
        category = zukan.CATEGORY_SOURCE.read_text(encoding="ascii")
        header = zukan.CATEGORY_SOURCE.parent / "internal/zukan_category.h"
        for name, end_name in (("CHARACTERS", "ZUKAN_CHARACTER_END"), ("WORLD_HISTORY", "ZUKAN_WORLD_HISTORY_END")):
            index = c_define(header, f"ZUKAN_CATEGORY_{name}")
            self.assertEqual(zukan.CATEGORY_END_OVERRIDES[index], c_define(zukan.CATEGORY_SOURCE, end_name))
            self.assertRegex(category, rf"category == ZUKAN_CATEGORY_{name}\)\s*\{{\s*category_ranges\[category \+ 1\] = {end_name};")
        for value, name in (
            (zukan.DISPLAY_ORDER_END, "ZUKAN_DISPLAY_ORDER_END"),
            (zukan.TECHNIQUE_EXTRA_COUNT, "ZUKAN_EXTRA_TECHNIQUE_COUNT"),
            (zukan.TECHNIQUE_EXTRA_FIRST, "ZUKAN_CHARACTER_END"),
            (zukan.HISTORY_GROUP_COUNT, "ZUKAN_HISTORY_GROUP_COUNT"),
            (zukan.HISTORY_GROUP_BASE, "ZUKAN_WORLD_HISTORY_START"),
            (zukan.HISTORY_GROUP_FIRST_BIT, "ZUKAN_HISTORY_GROUP_UNLOCK_BIT"),
        ):
            self.assertEqual(value, c_define(zukan.CATEGORY_SOURCE, name))
        self.assertEqual(zukan.TECHNIQUE_CATEGORY, c_define(header, "ZUKAN_CATEGORY_TECHNIQUES"))
        self.assertIn("display_order[i] != ZUKAN_DISPLAY_ORDER_END", category)
        self.assertIn("category == ZUKAN_CATEGORY_TECHNIQUES", category)
        self.assertIn("i < count + ZUKAN_EXTRA_TECHNIQUE_COUNT", category)
        self.assertIn("i + ZUKAN_CHARACTER_END - count", category)
        self.assertIn("j < ZUKAN_HISTORY_GROUP_COUNT", category)
        self.assertIn("group_ranges[j] + ZUKAN_WORLD_HISTORY_START", category)
        self.assertIn("ZUKAN_ENTRY_UNLOCKED(j + ZUKAN_HISTORY_GROUP_UNLOCK_BIT)", category)

    def test_table_lengths_follow_changed_counts_and_reject_unknown_names(self):
        source = zukan.CATEGORY_SOURCE.read_text(encoding="ascii")
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "category.c"
            path.write_text(source.replace("#define ZUKAN_HISTORY_GROUP_COUNT 6", "#define ZUKAN_HISTORY_GROUP_COUNT 8"), encoding="ascii")
            with patch.object(zukan, "CATEGORY_SOURCE", path):
                self.assertEqual(zukan.table_lengths()["group_ranges"], 9)
                path.write_text(source.replace("values[ZUKAN_CATEGORY_RANGE_COUNT]", "values[UNKNOWN_COUNT]"), encoding="ascii")
                with self.assertRaisesRegex(ValueError, "unsupported table length: UNKNOWN_COUNT"):
                    zukan.table_lengths()


if __name__ == "__main__":
    unittest.main()
