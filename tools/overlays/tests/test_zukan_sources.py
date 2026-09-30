"""Keep ZUKAN's extractor aligned with the regional maps and the C code."""

import re
import unittest
from pathlib import Path

from tools.overlays import cload, splat_config, zukan
from tools.overlays.tests.test_addhero_sources import c_define

REPO_ROOT = Path(__file__).resolve().parents[3]


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
        self.assertIn(f"while (i < 0x{zukan.SPRITE_COUNT:X});", source)
        self.assertIn(f"x + {zukan.SPRITE_SCREEN_X_OFFSET}, y", source)
        self.assertIn("(sprite_record->texture >> 14) & 0x1FF, sprite_record->texture >> 23", source)
        self.assertIn("((sprite_record->texture >> 8) & 0x3F) | 0x7C80", source)
        category = zukan.CATEGORY_SOURCE.read_text(encoding="ascii")
        for index, end in zukan.CATEGORY_END_OVERRIDES.items():
            self.assertRegex(category, rf"category == {index}\)\s*\{{\s*category_ranges\[category \+ 1\] = 0x{end:X};")
        self.assertIn(f"while (display_order[i] != 0x{zukan.DISPLAY_ORDER_END:X})", category)
        self.assertIn(f"category == 0x{zukan.TECHNIQUE_CATEGORY:X}", category)
        self.assertIn(f"i < count + 0x{zukan.TECHNIQUE_EXTRA_COUNT:X}", category)
        self.assertIn(f"count - 0x{zukan.TECHNIQUE_EXTRA_FIRST:X}", category)
        self.assertIn(f"while (j < {zukan.HISTORY_GROUP_COUNT})", category)
        self.assertIn(f"group_ranges[j] + 0x{zukan.HISTORY_GROUP_BASE:X}", category)
        self.assertIn(f"(j + 0x{zukan.HISTORY_GROUP_FIRST_BIT:X}) / 32", category)


if __name__ == "__main__":
    unittest.main()
