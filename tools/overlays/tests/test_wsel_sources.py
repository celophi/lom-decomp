"""Check WSEL's extractor against the regional maps and C layouts."""

import re
import unittest
from pathlib import Path

from tools.overlays import splat_config, wsel
from tools.overlays.tests.test_addhero_sources import c_define

REPO_ROOT = Path(__file__).resolve().parents[3]


class SourceTest(unittest.TestCase):
    def test_both_versions_have_one_blob_and_all_resource_boundaries(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = wsel.WselSymbols.load(config / wsel.SYMBOL_FILE)
                files = splat_config.data_files(config / wsel.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
                self.assertEqual(len(files), 1)
                source = files[0]
                self.assertEqual((source.name, source.kind), ("wsel_data", "databin"))
                self.assertEqual(source.start, names.land_map)
                addresses = [getattr(names, key) for key in wsel.SYMBOL_NAMES]
                self.assertEqual(addresses, sorted(set(addresses)))
                self.assertTrue(all(source.contains(address) for address in addresses))
                cells = wsel.MAP_CELLS * wsel.MAP_CELLS
                self.assertEqual(names.tile_masks - names.occupied, cells + 3)
                self.assertEqual(names.sprites - names.tile_masks, cells * wsel.SUBCELLS * wsel.SUBCELLS)
                self.assertEqual(names.poses_default - names.sprites, wsel.SPRITE_COUNT * wsel.SPRITE_RECORD.size)
                pose_table = wsel.HERO_POSE_COUNT * wsel.POSE_RECORD.size
                self.assertEqual(names.poses_alternate - names.poses_default, pose_table)
                self.assertEqual(names.variables - names.poses_alternate, pose_table)
                self.assertEqual(source.end - names.variables, 0x4074)

    def test_counts_follow_the_c_definitions(self):
        self.assertEqual(wsel.MAP_CELLS, c_define(wsel.SOURCE, "WSEL_MAP_CELLS"))
        self.assertEqual(wsel.SUBCELLS, c_define(wsel.SOURCE, "WSEL_SUBCELLS"))
        self.assertEqual(wsel.CELL_SIZE, c_define(wsel.SOURCE, "WSEL_CELL_SIZE"))
        self.assertEqual(wsel.SPRITE_COUNT, c_define(wsel.SOURCE, "WSEL_SPRITE_COUNT"))
        self.assertEqual(wsel.HERO_POSE_COUNT, c_define(wsel.SOURCE, "WSEL_HERO_POSE_COUNT"))

    def test_record_layouts_follow_the_c_structs(self):
        source = wsel.SOURCE.read_text(encoding="ascii")
        sprite = re.search(r"typedef struct\s*\{([^}]+)\}\s*WselSprite;", source).group(1)
        fields = re.findall(r"\b(u8|u16)\s+(\w+);", sprite)
        self.assertEqual(tuple(name for _, name in fields), wsel.SPRITE_FIELDS)
        self.assertEqual("".join("B" if kind == "u8" else "H" for kind, _ in fields), "BBBBHHHHHHHHHH")
        self.assertEqual(wsel.SPRITE_RECORD.size, 24)
        pose = re.search(r"typedef struct\s*\{([^}]+)\}\s*WselHeroPose;", source).group(1)
        fields = re.findall(r"\b(u8)\s+(\w+);", pose)
        self.assertEqual(tuple(name for _, name in fields), wsel.POSE_FIELDS)
        self.assertEqual(wsel.POSE_RECORD.size, 6)

    def test_layer_names_and_uploads_come_from_wsel_c(self):
        names = wsel.sprite_layer_names()
        self.assertEqual(set(names), set(range(wsel.SPRITE_COUNT)))
        self.assertEqual([names[index].removeprefix("WSEL_SPRITE_").lower() for index in range(8)], list(wsel.TIM_KEYS))
        source = wsel.SOURCE.read_text(encoding="ascii")
        uploads = dict(re.findall(r"wsel_upload_tim\((g_wsel_\w+), (WSEL_SPRITE_\w+)\);", source))
        self.assertEqual(uploads, {wsel.SYMBOL_NAMES[key]: names[index] for index, key in enumerate(wsel.TIM_KEYS)})


if __name__ == "__main__":
    unittest.main()
