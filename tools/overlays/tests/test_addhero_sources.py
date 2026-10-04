"""Check the ADDHERO extractor against the repository's own config and C sources.

These tests read no game data. They fail when a symbol the extractor needs is
renamed, or when a value it copies from C changes, and the message says what
to update.
"""

import re
import unittest
from pathlib import Path

from tools.overlays import addhero, card_data, splat_config

REPO_ROOT = Path(__file__).resolve().parents[3]
VERSIONS = ("us", "jp")

# AddheroSymbols fields that must point inside the data blob.
IN_BLOB = ("locations", "icon_offsets", "card_steps", "chart", "decimal_glyphs", "hex_glyphs")

# Each Python constant, and the C #define it mirrors.
C_CONSTANTS = (
    ("CHART_ROW_BYTES", "include/common/glyph_cache.h", "GLYPH_CHART_ROW_BYTES"),
    ("CHART_COLUMNS", "include/common/glyph_cache.h", "GLYPH_CHART_COLUMNS"),
    ("CHART_PAGE_BYTES", "include/common/glyph_cache.h", "GLYPH_CHART_PAGE_BYTES"),
    ("CHART_FIRST_CODE", "include/common/glyph_cache.h", "GLYPH_TEXT_FIRST_PRINTABLE"),
    ("ICON_HERO_COUNT", "include/common/saved_game.h", "SAVE_ICON_HERO_COUNT"),
    ("ICON_PET_BASE", "include/common/saved_game.h", "SAVE_ICON_PET_BASE"),
    ("ICON_GOLEM_BASE", "include/common/saved_game.h", "SAVE_ICON_GOLEM_BASE"),
)


def c_define(path: Path, name: str) -> int:
    """Evaluate an integer #define, following other #defines from the same file."""
    text = path.read_text(encoding="ascii")
    match = re.search(rf"^#define {name}\s+(.+?)\s*(?:/\*.*)?$", text, re.MULTILINE)
    if match is None:
        raise AssertionError(f"{path} has no #define {name}")
    expression = re.sub(
        r"\b[A-Z_][A-Z0-9_]*\b", lambda word: str(c_define(path, word.group(0))), match.group(1)
    )
    if not re.fullmatch(r"[0-9xA-Fa-f\s()+*/-]+", expression):
        raise AssertionError(f"#define {name} is not a plain integer expression: {expression}")
    return int(eval(expression.replace("/", "//")))  # digits and operators only, checked above


class SymbolFileTest(unittest.TestCase):
    def test_every_required_symbol_exists(self):
        for version in VERSIONS:
            with self.subTest(version=version):
                addhero.AddheroSymbols.load(REPO_ROOT / "config" / version / addhero.SYMBOL_FILE)

    def test_card_step_sequences_and_messages_have_names(self):
        for version in VERSIONS:
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = addhero.AddheroSymbols.load(config / addhero.SYMBOL_FILE)
                sequences = names.with_prefix(addhero.CARD_STEP_SYMBOL_PREFIX)
                messages = names.with_prefix(addhero.MESSAGE_SYMBOL_PREFIX)
                self.assertIn(names.card_steps, sequences.values())
                self.assertIn(names.messages, messages.values())


class SplatConfigTest(unittest.TestCase):
    def test_symbols_fall_in_the_data_files_the_extractor_reads(self):
        for version in VERSIONS:
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = addhero.AddheroSymbols.load(config / addhero.SYMBOL_FILE)
                assets = REPO_ROOT / "assets" / version
                files = splat_config.data_files(config / addhero.OVERLAY_CONFIG, assets)
                blob = splat_config.file_containing(files, names.messages, "messages")
                for key in IN_BLOB:
                    address = getattr(names, key)
                    self.assertTrue(blob.contains(address), f"{key} is outside the blob")
                for key in ("overflow_text", "file_template", "directory_pattern"):
                    source = splat_config.file_containing(files, getattr(names, key), key)
                    self.assertEqual(source.kind, "rodatabin", key)


class CSourceTest(unittest.TestCase):
    def test_constants_match_the_c_defines(self):
        for attribute, source, define in C_CONSTANTS:
            with self.subTest(constant=attribute):
                expected = c_define(REPO_ROOT / source, define)
                self.assertEqual(
                    getattr(card_data, attribute),
                    expected,
                    f"{define} in {source} is {expected}; "
                    f"update {attribute} in tools/overlays/card_data.py",
                )

    def test_card_step_names_come_from_the_header(self):
        names = addhero.card_step_names()
        self.assertEqual(names[0], "ADDHERO_STEP_DONE")
        self.assertEqual(names[14], "ADDHERO_STEP_WAIT")


if __name__ == "__main__":
    unittest.main()
