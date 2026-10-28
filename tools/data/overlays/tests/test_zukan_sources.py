"""Keep ZUKAN's extractor aligned with the regional maps and the C code."""

import re
import unittest
from pathlib import Path

from tools.data.overlays import cload, splat_config, zukan, zukan_initializers
from tools.data.overlays.tests.test_addhero_sources import c_define

REPO_ROOT = Path(__file__).resolve().parents[4]
DISPLAY_HEADER = REPO_ROOT / "include/main/display.h"


class SourceTest(unittest.TestCase):
    def test_regions_have_table_rodata_and_one_data_blob(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                overlay = config / zukan.OVERLAY_CONFIG
                names = zukan.ZukanSymbols.load(config / zukan.SYMBOL_FILE)
                files = splat_config.data_files(overlay, REPO_ROOT / "assets" / version)
                self.assertEqual([(f.name, f.kind) for f in files], [("zukan_data", "databin")])
                start, end = splat_config.section(overlay, *zukan.TABLE_SECTION)
                table_addresses = [getattr(names, key) for key in zukan.TABLE_KEYS]
                self.assertEqual(table_addresses, sorted(set(table_addresses)))
                self.assertEqual(start, names.category_ranges)
                self.assertTrue(all(start <= address < end for address in table_addresses))
                self.assertEqual(end, zukan.symbols.load(config / zukan.SYMBOL_FILE)["zukan_run"])
                data = files[0]
                data_addresses = [getattr(names, key) for key in zukan.DATA_KEYS]
                self.assertEqual(data_addresses, sorted(set(data_addresses)))
                self.assertEqual(data.start, names.archive)
                self.assertTrue(all(data.contains(address) for address in data_addresses))
                self.assertGreaterEqual(names.variables - names.sprites, zukan.SPRITE_COUNT * zukan.SPRITE_RECORD.size)
                if version == "jp":
                    chart = cload.CloadSymbols.load(config / cload.SYMBOL_FILE)
                    self.assertLess(chart.chart, chart.decimal_glyphs)

    def test_category_arrays_include_the_generated_lists(self):
        category = zukan.CATEGORY_SOURCE.read_text(encoding="ascii")
        types = {"i": "s32", "h": "s16", "H": "u16"}
        declared = re.findall(r"^    (\w+) (\w+)\[\] = \{\n#include \"zukan_category/(\w+)\.inc\"\n    \};$", category, re.MULTILINE)
        self.assertEqual(declared, [(types[element], name, name) for _, name, element in zukan_initializers.ARRAYS])
        self.assertEqual([key for key, _, _ in zukan_initializers.ARRAYS], list(zukan.TABLE_KEYS))
        self.assertEqual(len(zukan.ARCHIVE_SECTIONS), 4)

    def test_constants_follow_the_c_code(self):
        self.assertEqual(zukan.ENTRY_RESOURCE_BASE, c_define(zukan.SOURCE, "ZUKAN_ENTRY_RESOURCE_BASE"))
        source = zukan.SOURCE.read_text(encoding="ascii")
        record = re.search(r"typedef struct\s*\{([^}]+)\}\s*ZukanUiSpriteRecord;", source).group(1)
        fields = [(kind, name, int(bits) if bits else None)
                  for kind, name, bits in re.findall(r"\b(u32|u16)\s+(\w+)(?:\s*:\s*(\d+))?;", record)]
        self.assertEqual(fields, [
            ("u32", "image", 8), ("u32", "u", 24), ("u32", "v", 8), ("u32", "clut_column", 6),
            ("u32", "width", 9), ("u32", "height", 9), ("u16", "x", None), ("u16", "y", None)])
        self.assertEqual(zukan.SPRITE_RECORD.size, 12)
        # Pack each C bitfield at its declared position and check the extractor decodes the same values.
        words = [0, 0]
        word, bit = 0, 0
        values = {"image": 1, "u": 0xA8, "v": 0xE8, "clut_column": 0x2A, "width": 0x1A5, "height": 0x15A}
        for _, name, bits in fields[:6]:
            words[word] |= values[name] << bit
            bit += bits
            if bit == 32:
                word, bit = word + 1, 0
        sprite = zukan.Sprite(0, words[0], words[1], 0, 0)
        self.assertEqual((sprite.draw_mode, sprite.u, sprite.v, sprite.palette, sprite.width, sprite.height),
                         tuple(values[name] for name in ("image", "u", "v", "clut_column", "width", "height")))
        self.assertEqual(zukan.SPRITE_COUNT, c_define(zukan.SOURCE, "ZUKAN_UI_SPRITE_COUNT"))
        self.assertIn("i < ZUKAN_UI_SPRITE_COUNT", source)
        self.assertIn(f"x + {zukan.SPRITE_SCREEN_X_OFFSET}, y", source)
        self.assertIn("setWH(sprite, sprite_record->width, sprite_record->height)", source)
        self.assertIn("setClut(sprite, sprite_record->clut_column * GPU_CLUT_4BIT_COLORS, VRAM_CLUT_Y)", source)
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


if __name__ == "__main__":
    unittest.main()
