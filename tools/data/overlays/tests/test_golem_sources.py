"""Check GOLEM's extractor against the regional maps and C layouts."""

import re
import unittest
from pathlib import Path

from tools.data.overlays import cload, golem, splat_config
from tools.data.overlays.tests.test_addhero_sources import c_define

REPO_ROOT = Path(__file__).resolve().parents[4]


class SourceTest(unittest.TestCase):
    def test_both_versions_have_one_blob_and_all_resource_boundaries(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = golem.GolemSymbols.load(config / golem.SYMBOL_FILE)
                files = splat_config.data_files(config / golem.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
                self.assertEqual(len(files), 1)
                source = files[0]
                self.assertEqual((source.name, source.kind), ("golem_data", "databin"))
                self.assertEqual(source.start, names.image)
                addresses = [getattr(names, key) for key in golem.SYMBOL_NAMES]
                self.assertEqual(addresses, sorted(set(addresses)))
                self.assertTrue(all(source.contains(address) for address in addresses))
                self.assertEqual(names.panels - names.glyphs, golem.GLYPH_COUNT * golem.GLYPH_RECORD.size)
                self.assertEqual(names.variables - names.panels, golem.PANEL_COUNT * golem.PANEL_RECORD.size)
                self.assertEqual(source.end - names.variables, 292)
                if version == "jp":
                    chart = cload.CloadSymbols.load(config / cload.SYMBOL_FILE)
                    self.assertLess(chart.chart, chart.decimal_glyphs)

    def test_layouts_and_panel_count_follow_the_c_definitions(self):
        self.assertEqual(golem.PANEL_COUNT, c_define(golem.SOURCE, "GOLEM_PANEL_COUNT"))
        source = golem.SOURCE.read_text(encoding="ascii")
        glyph = re.search(r"typedef struct\s*\{([^}]+)\}\s*GolemGlyphMetric;", source).group(1)
        self.assertEqual(len(re.findall(r"\bu8\s+\w+;", glyph)), 4)
        self.assertEqual(len(re.findall(r"\bu16\s+\w+;", glyph)), 2)
        self.assertEqual(golem.GLYPH_RECORD.size, 8)
        panel = re.search(r"typedef struct\s*\{([^}]+)\}\s*GolemPanelRecord;", source).group(1)
        self.assertEqual(len(re.findall(r"\bu32\s+\w+;", panel)), 2)
        self.assertIn("GolemPanelDimensions dimensions;", panel)
        self.assertEqual(len(re.findall(r"\bu16\s+\w+;", panel)), 4)
        self.assertEqual(golem.PANEL_RECORD.size, 20)

    def test_panel_behaviors_come_from_the_owning_enum(self):
        names = golem.panel_behavior_names()
        self.assertEqual(set(names), set(range(8)))
        self.assertEqual(names[0], "GOLEM_PANEL_ALWAYS")
        self.assertEqual(names[6], "GOLEM_PANEL_FLASH_WHITE")


if __name__ == "__main__":
    unittest.main()
