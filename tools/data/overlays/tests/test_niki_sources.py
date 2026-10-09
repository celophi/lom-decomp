"""Check NIKI's extractor against both regional configs and the C sources.

These tests read no game data. They fail when a symbol the extractor needs is
renamed, when the yaml layout changes, or when a value it relies on changes
in C, and the message says what to update.
"""

import re
import unittest
from pathlib import Path

from tools.data.overlays import card_data, icon_set, niki, splat_config
from tools.data.overlays.tests.test_addhero_sources import c_define

REPO_ROOT = Path(__file__).resolve().parents[4]
VERSIONS = ("us", "jp")
HEADER = REPO_ROOT / "src/overlays/niki/internal/niki_internal.h"
CARD_MENU_HEADER = REPO_ROOT / "include/common/card_menu.h"

# Each card_data constant, and the NIKI #define that must agree with it.
C_CONSTANTS = (
    ("CHART_COLUMNS", "NIKI_SJIS_CODES_PER_ROW"),
    ("CHART_ROWS_PER_PAGE", "NIKI_SJIS_ROWS_PER_PAGE"),
    ("CHART_FIRST_CODE", "NIKI_TEXT_SINGLE_BYTE_BASE"),
)


def load(version: str) -> tuple[niki.NikiSymbols, list[splat_config.DataFile]]:
    config = REPO_ROOT / "config" / version
    names = niki.NikiSymbols.load(config / niki.SYMBOL_FILE)
    files = splat_config.data_files(config / niki.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
    return names, files


class SourceTest(unittest.TestCase):
    def test_both_versions_have_one_blob_in_resource_order(self):
        for version in VERSIONS:
            with self.subTest(version=version):
                names, files = load(version)
                self.assertEqual([(source.name, source.kind) for source in files], [("niki_data", "databin")])
                source = files[0]
                self.assertEqual(source.start, names.messages)
                addresses = [getattr(names, key) for key in niki.BLOB_ORDER]
                self.assertEqual(addresses, sorted(set(addresses)))
                self.assertTrue(all(source.contains(address) for address in addresses))
                self.assertTrue(names.messages < names.chart_pages < names.chart)

    def test_blob_runs_to_the_segment_end_with_no_bss_after_it(self):
        for version in VERSIONS:
            with self.subTest(version=version):
                names, files = load(version)
                source = files[0]
                last = max(address for address in names.named.values() if source.contains(address))
                self.assertEqual(last, names.named["g_glyph_cursor_y"])
                self.assertEqual(source.end, last + 4)

    def test_text_and_step_symbols_are_named(self):
        for version in VERSIONS:
            with self.subTest(version=version):
                names, _ = load(version)
                slots = names.with_prefix(niki.MESSAGE_SYMBOL_PREFIX)
                self.assertIn(names.messages, slots.values())
                self.assertTrue(all(names.messages <= address < names.locations for address in slots.values()))
                self.assertTrue(all((address - names.messages) % 2 == 0 for address in slots.values()))
                steps = [a for a in names.named.values() if names.card_steps <= a < names.chart]
                self.assertEqual(len(steps), 8)
                self.assertIn(names.named["g_card_steps_idle"], steps)

    def test_chart_constants_match_the_niki_defines(self):
        for attribute, define in C_CONSTANTS:
            with self.subTest(constant=attribute):
                self.assertEqual(getattr(card_data, attribute), c_define(HEADER, define))
        self.assertEqual(card_data.CHART_LEADS.start, c_define(HEADER, "NIKI_TEXT_EXTENDED_LEAD_FIRST"))
        self.assertEqual(len(card_data.CHART_LEADS), c_define(HEADER, "NIKI_TEXT_EXTENDED_PAGE_COUNT"))

    def test_icon_layout_matches_card_menu_icon_image(self):
        header = CARD_MENU_HEADER.read_text(encoding="ascii")
        body = re.search(r"typedef struct\s*\{([^}]+)\}\s*CardMenuIconImage;", header).group(1)
        palette = re.search(r"u16 clut\[(\d+)\];", body)
        icon_size = c_define(CARD_MENU_HEADER, "CARD_MENU_ICON_SIZE")
        self.assertRegex(body, r"u8 pixels\[CARD_MENU_ICON_SIZE \* CARD_MENU_ICON_SIZE / 2\];")
        self.assertEqual(int(palette[1]), icon_set.PALETTE_COLORS)
        self.assertEqual(icon_size, icon_set.ICON_SIZE)
        self.assertEqual(icon_size * icon_size // 2, icon_set.PIXEL_BYTES)

    def test_command_names_come_from_the_header(self):
        names = niki.card_step_names()
        self.assertEqual(names[0], "NIKI_COMMAND_STOP")
        self.assertEqual(names[30], "NIKI_COMMAND_RESET_RETRIES")
        self.assertNotIn(14, names)


if __name__ == "__main__":
    unittest.main()
