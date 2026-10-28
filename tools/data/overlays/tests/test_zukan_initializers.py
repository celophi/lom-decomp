"""Check the initializer macros written for zukan_category.c with synthetic data and no disc files."""

import re
import tempfile
import unittest
from pathlib import Path

from tools.data.overlays import zukan_initializers
from tools.data.overlays.tests.test_zukan_extract import ENTRY_COUNT, FakeOverlay


class InitializerTest(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        self.inputs = FakeOverlay().write(self.root)
        self.assets = self.root / "generated"
        self.header = self.assets / zukan_initializers.HEADER

    def macros(self):
        text = self.header.read_text(encoding="ascii")
        bodies = dict(re.findall(r"^#define (\w+) \\\n((?:.*\\\n)*.*)$", text, re.MULTILINE))
        return {name: [int(value) for value in body.replace("\\\n", " ").split(",")] for name, body in bodies.items()}

    def test_macros_hold_each_table_in_rodata_order(self):
        zukan_initializers.write_initializers(self.inputs.config, "us", self.assets)
        macros = self.macros()
        self.assertEqual(list(macros), [macro for *_, macro in zukan_initializers.ARRAYS])
        self.assertEqual(macros["ZUKAN_CATEGORY_RANGES"], [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13])
        self.assertEqual(macros["ZUKAN_GROUP_RANGES"], [0, 1, 2, 3, 4, 5, 6])
        self.assertEqual(macros["ZUKAN_RESOURCE_VALUES"], [0 if i % 5 == 0 else i for i in range(ENTRY_COUNT)])
        text = self.header.read_text(encoding="ascii")
        self.assertTrue(text.startswith(f"#ifndef {zukan_initializers.GUARD}\n#define {zukan_initializers.GUARD}\n"))
        self.assertTrue(all(line.count(",") <= zukan_initializers.VALUES_PER_LINE for line in text.splitlines()))

    def test_display_order_stops_at_its_terminator(self):
        zukan_initializers.write_initializers(self.inputs.config, "us", self.assets)
        self.assertEqual(self.macros()["ZUKAN_DISPLAY_ORDER"], [2, 1, 1024])

    def test_missing_overlay_image_writes_nothing(self):
        (self.root / "build/ZUKAN.BIN").unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            zukan_initializers.write_initializers(self.inputs.config, "us", self.assets)
        self.assertFalse(self.header.exists())


if __name__ == "__main__":
    unittest.main()
