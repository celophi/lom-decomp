"""Keep GNAME's extractor in step with its source and regional layouts."""

import unittest
from pathlib import Path

import yaml

from tools.data.overlays import cload, gname, splat_config
from tools.data.overlays.tests.test_addhero_sources import c_define

REPO_ROOT = Path(__file__).resolve().parents[4]


class SourceTest(unittest.TestCase):
    def test_both_regions_have_one_blob_followed_by_the_original_bss(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = gname.GnameSymbols.load(config / gname.SYMBOL_FILE)
                files = splat_config.data_files(config / gname.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
                self.assertEqual(len(files), 1)
                source = files[0]
                self.assertEqual((source.name, source.kind), ("gname_data", "databin"))
                self.assertEqual(source.start, names.panels)
                self.assertEqual(source.end, names.variables)
                addresses = [getattr(names, key) for key in gname.SYMBOL_NAMES]
                self.assertEqual(addresses, sorted(set(addresses)))
                self.assertEqual(names.categories - names.panels, gname.PANEL_BOUNDARY_COUNT * 4)
                self.assertEqual(names.cursors - names.glyphs, gname.GLYPH_COUNT * 8)
                self.assertEqual(names.kanji_offsets - names.cursors, gname.CURSOR_COUNT * 4)
                self.assertEqual(names.records - names.kanji_offsets, gname.KANJI_BOUNDARY_COUNT * 4)
                self.assertEqual(names.animation - names.layout, gname.LAYOUT_SPRITE_COUNT * 8)
                self.assertEqual(names.glyphs - names.categories, (10 if version == "us" else 50) * 4)
                animation_size = gname.ANIMATION_FRAME_COUNT * gname.ANIMATION_SLOT_COUNT * 4
                self.assertEqual(names.variables - names.animation, animation_size + (4 if version == "us" else 0))
                document = yaml.safe_load((config / gname.OVERLAY_CONFIG).read_text())
                self.assertEqual(document["segments"][0]["subsegments"][-1][1:], [".bss", "gname"])
                if version == "jp":
                    chart = cload.CloadSymbols.load(config / cload.SYMBOL_FILE)
                    self.assertLess(chart.chart, chart.decimal_glyphs)

    def test_counts_follow_the_c_definitions(self):
        source = REPO_ROOT / "src/overlays/gname/gname.c"
        for value, name in (
            (gname.LAYOUT_SPRITE_COUNT, "GNAME_LAYOUT_SPRITE_COUNT"),
            (gname.ANIMATION_FRAME_COUNT, "GLYPH_APPEND_ANIM_FRAME_COUNT"),
            (gname.ANIMATION_SLOT_COUNT, "GLYPH_APPEND_ANIM_SLOT_COUNT"),
            (gname.UNMAPPED_CATEGORY, "KANJI_CATEGORY_EMPTY"),
            (gname.CURSOR_COUNT, "GNAME_SELECTION_ENTRY_END_EXCLUSIVE"),
        ):
            with self.subTest(name=name):
                self.assertEqual(value, c_define(source, name))


if __name__ == "__main__":
    unittest.main()
