"""Check MENU's extractor against the regional maps and C layouts."""

import re
import unittest
from pathlib import Path

from tools.overlays import cload, menu, splat_config
from tools.overlays.tests.test_addhero_sources import c_define

REPO_ROOT = Path(__file__).resolve().parents[3]
TIM_HEADER = REPO_ROOT / "include/tim.h"
MENU_HEADER = REPO_ROOT / "include/menu.h"
SCREENS = REPO_ROOT / "src/overlays/menu/menu_screens.c"


def struct_body(path: Path, name: str) -> str:
    source = path.read_text(encoding="ascii")
    match = re.search(r"typedef struct\s*\{([^}]+)\}\s*" + name + ";", source)
    if match is None:
        raise AssertionError(f"{path} has no struct {name}")
    return match.group(1)


class SourceTest(unittest.TestCase):
    def test_both_versions_have_one_blob_after_the_alignment_rodata(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = menu.MenuSymbols.load(config / menu.SYMBOL_FILE)
                files = splat_config.data_files(config / menu.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
                self.assertEqual([(f.name, f.kind) for f in files], [
                    ("menu_scene_rodata_alignment", "rodatabin"),
                    ("menu_actions_rodata_alignment", "rodatabin"),
                    ("menu_data", "databin"),
                ])
                source = files[-1]
                self.assertEqual(source.start, names.icon_sprites)
                addresses = [getattr(names, key) for key in menu.SYMBOL_NAMES]
                self.assertEqual(addresses, sorted(set(addresses)))
                self.assertTrue(all(source.contains(address) for address in addresses))
                bases = [names.named[name] for name in menu.LABEL_BASES]
                self.assertEqual(bases, sorted(set(bases)))
                self.assertTrue(names.image < bases[0] and bases[-1] < names.group_ids)
                if version == "jp":
                    chart = cload.CloadSymbols.load(config / cload.SYMBOL_FILE)
                    self.assertLess(chart.chart, chart.decimal_glyphs)

    def test_record_counts_fit_the_symbol_distances(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                names = menu.MenuSymbols.load(REPO_ROOT / "config" / version / menu.SYMBOL_FILE)
                icons = (names.icon_palettes - names.icon_sprites) // menu.ICON_RECORD.size
                self.assertEqual(icons, 113)
                self.assertLessEqual(names.icon_palettes + icons, names.item_counts)
                self.assertEqual(names.scripts + menu.SCRIPT_ROW.size, names.content_table + menu.NODE_COUNT * 4)
                self.assertEqual(names.grid_sprites - names.scripts, 17 * menu.SCRIPT_ROW.size)
                self.assertEqual(names.cursor_icons, names.grid_sprites + menu.GRID_SPRITE_COUNT * menu.GRID_SPRITE.size)
                self.assertEqual(names.content_table - names.group_ids, menu.NODE_COUNT)
                self.assertGreaterEqual(names.variables - names.cursor_icons, menu.CURSOR_FRAMES)

    def test_constants_follow_the_c_definitions(self):
        header = menu.HEADER
        for constant, name in (
            (menu.NODE_COUNT, "MENU_NODE_COUNT"),
            (menu.GRID_SPRITE_COUNT, "MENU_GRID_SPRITE_COUNT"),
            (menu.GRID_ALT_CLUT_START, "MENU_GRID_ALT_CLUT_START"),
            (menu.TIM_IMAGE_BLOCK_SIZE, "MENU_TIM_IMAGE_BLOCK_SIZE"),
            (menu.ICON_CLUT_Y_BASE, "MENU_ICON_CLUT_Y_BASE"),
            (menu.CONTENT_X_MASK, "MENU_CONTENT_X_MASK"),
            (menu.CONTENT_STYLE_SHIFT, "MENU_CONTENT_STYLE_SHIFT"),
            (menu.CONTENT_STYLE_MASK, "MENU_CONTENT_STYLE_MASK"),
            (menu.CONTENT_TYPE_SHIFT, "MENU_CONTENT_TYPE_SHIFT"),
        ):
            self.assertEqual(constant, c_define(header, name), name)
        self.assertEqual(menu.CLUT_ENTRY_COUNT, c_define(TIM_HEADER, "CLUT_ENTRY_COUNT"))
        self.assertEqual(menu.SCRIPT_END, c_define(MENU_HEADER, "MENU_SCRIPT_END"))
        self.assertEqual(menu.text_table_count(), 34)
        self.assertEqual(menu.text_table_names()[0], "MENU_TEXT_GENERAL")
        self.assertEqual(menu.content_type_names(), {0x5: "submenu", 0xF: "action"})
        self.assertEqual(menu.pad_button_names()[0x1000], "PADLup")
        self.assertEqual(menu.pad_button_names()[0x0800], "PADh")

    def test_record_layouts_follow_the_c_structs(self):
        icon = struct_body(menu.HEADER, "MenuIconSpriteInfo")
        self.assertEqual(len(re.findall(r"\bu8\s+\w+;", icon)), 4)
        self.assertEqual(menu.ICON_RECORD.size, 4)
        item = struct_body(menu.HEADER, "MenuContentItem")
        self.assertEqual(re.findall(r"\b(u8|u16)\s+(\w+)(\[\d+\])?;", item),
                         [("u16", "packed_x", ""), ("u8", "y", ""), ("u8", "action_type", ""), ("u8", "params", "[4]")])
        self.assertEqual(menu.CONTENT_ITEM.size, 8)
        grid = struct_body(menu.HEADER, "MenuGridSpriteDef")
        self.assertEqual(re.findall(r"\b(u16|u32)\s+\w+;", grid), ["u16", "u16", "u32", "u32"])
        self.assertEqual(menu.GRID_SPRITE.size, 12)
        script = struct_body(MENU_HEADER, "MenuScript")
        self.assertIn(f"u16 inputs[{menu.SCRIPT_INPUTS}];", script)
        asset = struct_body(menu.HEADER, "MenuTimAsset")
        self.assertEqual(re.findall(r"\bu32\s+(\w+);", asset), ["entry_count", "tim_offset", "second_clut_offset"])
        self.assertIn("u16 second_clut[CLUT_ENTRY_COUNT];", asset)

    def test_label_index_base_matches_the_scene_code(self):
        source = SCREENS.read_text(encoding="ascii")
        match = re.search(r"\(u32\)\(sub - (0x[0-9A-F]+)\) < 0x1E\)\s*\{\s*t0 = D_80168659\[", source)
        self.assertIsNotNone(match)
        self.assertEqual(int(match.group(1), 16), menu.ACTION_LABEL_FIRST)


if __name__ == "__main__":
    unittest.main()
