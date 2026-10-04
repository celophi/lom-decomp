"""Check SHOP's extractor against the regional maps and C layouts."""

import re
import unittest
from pathlib import Path

from tools.overlays import cload, shop, splat_config
from tools.overlays.tests.test_addhero_sources import c_define

REPO_ROOT = Path(__file__).resolve().parents[3]
SAVED_GAME = REPO_ROOT / "include/common/saved_game.h"


class SourceTest(unittest.TestCase):
    def test_both_versions_have_one_blob_and_all_resource_boundaries(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = shop.ShopSymbols.load(config / shop.SYMBOL_FILE)
                files = splat_config.data_files(config / shop.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
                self.assertEqual(len(files), 1)
                source = files[0]
                self.assertEqual((source.name, source.kind), ("shop_data", "databin"))
                self.assertEqual(source.start, names.text)
                addresses = [getattr(names, key) for key in shop.SYMBOL_NAMES]
                self.assertEqual(addresses, sorted(set(addresses)))
                self.assertTrue(all(source.contains(address) for address in addresses))
                self.assertEqual(names.variables - names.prices, shop.ITEM_KIND_COUNT * 2 + 4)
                self.assertEqual(source.end - names.variables, 0xB2C)
                if version == "jp":
                    chart = cload.CloadSymbols.load(config / cload.SYMBOL_FILE)
                    self.assertLess(chart.chart, chart.decimal_glyphs)

    def test_counts_follow_the_c_definitions(self):
        self.assertEqual(shop.ITEM_KIND_COUNT, c_define(SAVED_GAME, "FIELD_ITEM_KIND_COUNT"))
        self.assertEqual(shop.MATERIAL_COUNT, c_define(SAVED_GAME, "FIELD_ITEM_MATERIAL_MASK") + 1)
        self.assertEqual(shop.SPELLS_PER_SPIRIT, c_define(shop.RENDER_SOURCE, "SPELLS_PER_SPIRIT"))
        for category, first in shop.CATEGORY_FIRSTS:
            self.assertEqual(first, c_define(shop.RENDER_SOURCE, f"{category.upper()}_CATEGORY_FIRST"))
        header = shop.HEADER.read_text(encoding="ascii")
        self.assertIn("extern u16 g_shop_item_sell_prices[FIELD_ITEM_KIND_COUNT];", header)

    def test_archive_header_follows_the_c_struct(self):
        header = shop.HEADER.read_text(encoding="ascii")
        body = re.search(r"typedef struct\s*\{([^}]+)\}\s*ShopTextArchive;", header).group(1)
        self.assertEqual(len(re.findall(r"\bu16\s+\w+;", body)), 2)
        self.assertIn("u32 section_offsets[SHOP_TEXT_SECTION_COUNT];", body)
        self.assertRegex(header, rf"SHOP_TEXT_SECTION_COUNT\s*=\s*{shop.SECTION_COUNT}\b")
        self.assertEqual(shop.ARCHIVE_HEADER.size, 4 + 4 * shop.SECTION_COUNT)

    def test_section_names_come_from_the_owning_enum(self):
        self.assertEqual(shop.text_section_names(), {
            0: "SHOP_TEXT_ITEM_DESCRIPTIONS", 1: "SHOP_TEXT_ITEM_NAMES",
            2: "SHOP_TEXT_EQUIPMENT_TYPES", 3: "SHOP_TEXT_INSTRUMENT_SPELLS",
        })


if __name__ == "__main__":
    unittest.main()
