"""Keep GOSUB's extractor aligned with the regional maps and C uploads."""

import unittest
from pathlib import Path

import yaml

from tools.data.overlays import gosub, splat_config
from tools.data.overlays.tests.test_addhero_sources import c_define

REPO_ROOT = Path(__file__).resolve().parents[4]


class SourceTest(unittest.TestCase):
    def test_regions_have_one_data_blob_one_group_blob_and_existing_bss(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = gosub.GosubSymbols.load(config / gosub.SYMBOL_FILE)
                files = splat_config.data_files(config / gosub.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
                self.assertEqual([(f.name, f.kind) for f in files], [("gosub_groups", "rodatabin"), ("gosub_data", "databin")])
                groups, data = files
                self.assertEqual(groups.start, names.group_firsts)
                self.assertEqual(groups.end, names.group_counts + 12)
                self.assertEqual(groups.end - groups.start, 24)
                self.assertEqual(data.start, names.image)
                self.assertEqual(data.end, names.bss)
                addresses = [getattr(names, key) for key in gosub.DATA_KEYS]
                self.assertEqual(addresses, sorted(set(addresses)))
                self.assertTrue(all(data.contains(address) for address in addresses))
                self.assertEqual(names.bss - names.glyphs, gosub.GLYPH_COUNT * gosub.GLYPH_RECORD.size + 4)
                document = yaml.safe_load((config / gosub.OVERLAY_CONFIG).read_text())
                self.assertEqual(document["segments"][0]["subsegments"][-1][1:], [".bss", "gosub"])
                font_end = names.font + gosub.FONT_PIXEL_OFFSET + gosub.FONT_WIDTH_WORDS * gosub.FONT_HEIGHT * 2
                self.assertEqual(font_end - names.colors, 92)
                self.assertEqual(names.colors + gosub.COLOR_FIRST - font_end, 4)

    def test_constants_follow_the_c_header(self):
        for value, name in (
            (gosub.COLOR_FIRST, "GOSUB_COLOR_MATERIAL_FIRST"),
            (gosub.COLOR_END, "GOSUB_COLOR_MATERIAL_END"),
            (gosub.FONT_PIXEL_OFFSET, "GOSUB_FONT_TEXTURE_DATA_OFFSET"),
            (gosub.FONT_WIDTH_WORDS, "GOSUB_FONT_TEXTURE_WIDTH"),
            (gosub.FONT_HEIGHT, "GOSUB_FONT_TEXTURE_HEIGHT"),
            (gosub.PORTRAIT_GOLEM_FIRST, "GOSUB_GOLEM_PORTRAIT_FIRST"),
            (gosub.PORTRAIT_EGG_FIRST, "GOSUB_EGG_PORTRAIT_FIRST"),
        ):
            with self.subTest(name=name):
                self.assertEqual(value, c_define(gosub.HEADER, name))

    def test_text_section_and_message_names_come_from_the_c_enums(self):
        sections = gosub.enum_names("GosubTextSection")
        self.assertEqual(sections[gosub.SECTION_COUNT], "GOSUB_TEXT_SECTION_COUNT")
        self.assertEqual(sections[11], "GOSUB_TEXT_COLOR_NAMES")
        messages = gosub.enum_names("GosubMessage")
        self.assertEqual(messages[0], "GOSUB_MSG_YES")
        self.assertEqual(messages[56], "GOSUB_MSG_CANNOT_LEAVE")


if __name__ == "__main__":
    unittest.main()
