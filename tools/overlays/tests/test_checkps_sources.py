"""Check CHECKPS's resource boundaries and source constants without disc files."""

import re
import unittest
from pathlib import Path

from tools.overlays import checkps, splat_config
from tools.overlays.tests.test_addhero_sources import c_define

REPO_ROOT = Path(__file__).resolve().parents[3]


class SourceTest(unittest.TestCase):
    def test_one_blob_holds_initialized_data_and_ends_at_bss(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = checkps.CheckpsSymbols.load(config / checkps.SYMBOL_FILE)
                files = splat_config.data_files(config / checkps.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
                data = [f for f in files if f.kind == "databin"]
                self.assertEqual(len(data), 1)
                self.assertEqual(data[0].name, "checkps_data")
                self.assertEqual(data[0].end, names.bss)
                for key in checkps.SYMBOL_NAMES.keys() - {"bss", "warning", "quadrant_signs"}:
                    self.assertTrue(data[0].contains(getattr(names, key)), key)
                for key in ("warning", "quadrant_signs"):
                    source = splat_config.file_containing(files, getattr(names, key), key)
                    self.assertEqual(source.kind, "rodatabin")

    def test_pattern_and_warning_constants_follow_c(self):
        for value, file, name in (
            (checkps.PATTERN_SIZE_COUNT, "pattern.c", "CHECKPS_PATTERN_SIZE_COUNT"),
            (checkps.PATTERN_QUADRANT_COUNT, "pattern.c", "CHECKPS_PATTERN_QUADRANT_COUNT"),
            (checkps.WARNING_BYTES, "internal/checkps_internal.h", "CHECKPS_HARDWARE_WARNING_SIZE"),
        ):
            with self.subTest(constant=name):
                self.assertEqual(value, c_define(REPO_ROOT / "src/overlays/checkps" / file, name))

    def test_font_prefix_size_follows_its_c_declaration(self):
        source = (REPO_ROOT / "src/overlays/checkps/font.c").read_text(encoding="ascii")
        words = int(re.search(r"g_glyph_clut_prefix\[(\d+)\]", source)[1])
        self.assertEqual(checkps.GLYPH_CLUT_PREFIX_BYTES, words * 4)

    def test_cd_names_come_from_the_source_enums(self):
        commands = checkps.enum_names("CheckPSCdCommandIndex")
        self.assertEqual(len(commands), 14)
        self.assertEqual(commands[13], "CHECKPS_CD_CMD_GET_ID")
        self.assertEqual(checkps.enum_names("CheckPSState")[0], "CHECKPS_STATE_IDLE")


if __name__ == "__main__":
    unittest.main()
