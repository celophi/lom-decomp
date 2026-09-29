"""Keep CLOAD's extractor in step with both regional layouts and the C decoder."""

import re
import unittest
from pathlib import Path

from tools.overlays import cload, splat_config

REPO_ROOT = Path(__file__).resolve().parents[3]


class SourceTest(unittest.TestCase):
    def test_both_versions_have_one_blob_and_all_resource_symbols(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = cload.CloadSymbols.load(config / cload.SYMBOL_FILE)
                files = splat_config.data_files(config / cload.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
                self.assertEqual(len(files), 1)
                self.assertEqual(files[0].name, "cload_data")
                self.assertEqual(files[0].kind, "databin")
                addresses = [getattr(names, key) for key in cload.SYMBOL_NAMES]
                self.assertEqual(addresses, sorted(set(addresses)))
                self.assertTrue(all(files[0].contains(address) for address in addresses))
                self.assertEqual(names.messages, files[0].start)
                self.assertTrue(all(
                    names.card_steps <= address < names.chart
                    for address in names.with_prefix(cload.CARD_STEP_SYMBOL_PREFIX).values()
                ))

    def test_chart_page_base_agrees_with_the_c_decoder(self):
        source = (REPO_ROOT / "src/overlays/cload/cload_glyph.c").read_text(encoding="ascii")
        branches = re.search(
            r"#if defined\(VERSION_JP\)(.*?)#else(.*?)#endif", source, re.S
        )
        for version, branch in zip(("jp", "us"), branches.groups()):
            with self.subTest(version=version):
                function, offset = re.search(
                    r"#define CLOAD_GLYPH_CHART_PAGE_BASE \(\(u8\*\)(\w+) \+ (0x[0-9A-Fa-f]+)\)", branch
                ).groups()
                names = cload.CloadSymbols.load(REPO_ROOT / "config" / version / cload.SYMBOL_FILE)
                self.assertEqual(names.chart_pages, names.named[function] + int(offset, 16))

    def test_step_names_follow_the_c_enum(self):
        names = cload.card_step_names()
        self.assertEqual(names[0], "CLOAD_STEP_DONE")
        self.assertEqual(names[14], "CLOAD_STEP_IDLE")
        self.assertEqual(names[30], "CLOAD_STEP_ARM_SAVE_RETRIES")


if __name__ == "__main__":
    unittest.main()
