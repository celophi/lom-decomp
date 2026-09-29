"""Check CARDA's extractor against both versions' config, without reading game data."""

import re
import unittest
from pathlib import Path

from tools.overlays import carda, splat_config

REPO_ROOT = Path(__file__).resolve().parents[3]


class SourceTest(unittest.TestCase):
    def test_both_versions_have_one_data_blob_and_all_required_symbols(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = carda.CardaSymbols.load(config / carda.SYMBOL_FILE)
                files = splat_config.data_files(config / carda.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
                blobs = [source for source in files if source.kind == "databin"]
                self.assertEqual(len(blobs), 1)
                self.assertEqual(blobs[0].name, "carda_data")
                for key in (
                    "messages", "items", "save_title", "bad_title", "save_icon_offsets",
                    "locations", "icon_offsets", "card_steps", "chart", "decimal_glyphs", "hex_glyphs",
                ):
                    self.assertTrue(blobs[0].contains(getattr(names, key)), key)
                for key in (
                    "title_dash", "title_colon", "save_card_path", "overflow_text", "card_path", "directory_pattern",
                ):
                    source = splat_config.file_containing(files, getattr(names, key), key)
                    self.assertEqual(source.kind, "rodatabin", key)
                sequences = names.with_prefix(carda.CARD_STEP_SYMBOL_PREFIX)
                self.assertIn(names.card_steps, sequences.values())

    def test_save_icon_dimensions_follow_the_c_struct(self):
        source = (REPO_ROOT / "src/overlays/carda/carda_widgets.c").read_text(encoding="ascii")
        layout = re.search(r"typedef struct\s*\{([^}]+)\} CardaSaveIcon;", source).group(1)
        frames, frame_bytes = map(int, re.search(r"frames\[(\d+)\]\[(\d+)\]", layout).groups())
        self.assertEqual(carda.SAVE_ICON_FRAMES, frames)
        self.assertEqual(carda.SAVE_ICON_FRAME_BYTES, frame_bytes)

    def test_step_names_follow_the_c_enum(self):
        names = carda.card_step_names()
        self.assertEqual(names[0], "CARDA_STEP_DONE")
        self.assertEqual(names[14], "CARDA_STEP_IDLE")
        self.assertEqual(names[30], "CARDA_STEP_ARM_RETRIES")


if __name__ == "__main__":
    unittest.main()
