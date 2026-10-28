"""Check the initializer lists written for zukan_category.c with synthetic data and no disc files."""

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
        self.overlay = FakeOverlay()
        self.inputs = self.overlay.write(self.root)
        self.output = self.root / "zukan_category"

    def read(self, name):
        return (self.output / f"{name}.inc").read_text(encoding="ascii")

    def test_lists_hold_each_table_in_rodata_order(self):
        zukan_initializers.write_initializers(self.inputs.config, "us", self.output)
        self.assertEqual(self.read("category_ranges"), "0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13,\n")
        self.assertEqual(self.read("group_ranges"), "0, 1, 2, 3, 4, 5, 6,\n")
        values = [int(value) for value in self.read("resource_values").replace("\n", " ").split(",") if value.strip()]
        self.assertEqual(values, [0 if i % 5 == 0 else i for i in range(ENTRY_COUNT)])
        self.assertTrue(all(len(line.split(",")) - 1 <= zukan_initializers.VALUES_PER_LINE
                            for line in self.read("resource_values").splitlines()))

    def test_display_order_stops_at_its_terminator(self):
        zukan_initializers.write_initializers(self.inputs.config, "us", self.output)
        self.assertEqual(self.read("display_order"), "2, 1, 1024,\n")

    def test_missing_overlay_image_writes_nothing(self):
        (self.root / "build/ZUKAN.BIN").unlink()
        with self.assertRaisesRegex(ValueError, "run make splat first"):
            zukan_initializers.write_initializers(self.inputs.config, "us", self.output)
        self.assertFalse(self.output.exists())


if __name__ == "__main__":
    unittest.main()
