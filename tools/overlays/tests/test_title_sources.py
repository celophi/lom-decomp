"""Keep TITLE's extractor in step with its C sources and both regional layouts."""

import re
import unittest
from pathlib import Path

import yaml

from tools.overlays import cload, saved_game, splat_config, title

REPO_ROOT = Path(__file__).resolve().parents[3]
TITLE_SOURCE = REPO_ROOT / "src/overlays/title/title.c"
MENU_STATE_SIZE = 0x8C  # g_title_menu_exit_state through g_slot_highlight_frames


def define(path: Path, name: str) -> int:
    """An integer #define, allowing a U suffix."""
    match = re.search(rf"^#define {name}\s+(0x[0-9A-Fa-f]+|\d+)U?\b", path.read_text(encoding="ascii"), re.MULTILINE)
    if match is None:
        raise AssertionError(f"{path} has no integer #define {name}")
    return int(match.group(1), 0)


class SourceTest(unittest.TestCase):
    def test_both_regions_have_one_blob_ending_at_the_segment_end(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                config = REPO_ROOT / "config" / version
                names = title.TitleSymbols.load(config / title.SYMBOL_FILE)
                files = splat_config.data_files(config / title.OVERLAY_CONFIG, REPO_ROOT / "assets" / version)
                self.assertEqual(len(files), 1)
                source = files[0]
                self.assertEqual((source.name, source.kind), ("title_data", "databin"))
                self.assertEqual(source.start, names.menu_offsets)
                self.assertEqual(source.end - names.variables, MENU_STATE_SIZE)
                addresses = [address for _, address in names.ordered()]
                self.assertEqual(addresses, sorted(set(addresses)))
                self.assertTrue(all(source.contains(address) for address in addresses))
                document = yaml.safe_load((config / title.OVERLAY_CONFIG).read_text())
                self.assertEqual(document["segments"][0]["subsegments"][-1][1:], ["databin", "title_data"])
                if version == "jp":
                    chart = cload.CloadSymbols.load(config / cload.SYMBOL_FILE)
                    self.assertLess(chart.chart, chart.decimal_glyphs)

    def test_fixed_size_resources_fill_their_symbols(self):
        for version in ("us", "jp"):
            with self.subTest(version=version):
                names = title.TitleSymbols.load(REPO_ROOT / "config" / version / title.SYMBOL_FILE)
                self.assertEqual(names.textures[0] - names.cursor_blink, title.CURSOR_FRAME_COUNT)
                self.assertEqual(names.panel_uvs - names.texture_table, title.TEXTURE_COUNT * 0x10)
                self.assertEqual(names.sprite_uvs - names.panel_uvs, (title.WEAPON_SLOT_COUNT + 1) * 6)
                self.assertEqual(names.layout - names.sprite_uvs, (title.WEAPON_SLOT_COUNT + 1) * 6)
                self.assertEqual(names.weapons - names.layout, title.LAYOUT_ENTRY_COUNT * 0x18)
                self.assertEqual(names.new_game - names.weapons, title.WEAPON_SLOT_COUNT * saved_game.ITEM_SIZE)
                self.assertEqual(names.alternate - names.new_game, title.NEW_GAME_SIZE)
                self.assertEqual(names.hero_default - names.alternate, saved_game.LAYOUT_SIZE)
                self.assertEqual(names.hero_continue - names.hero_default, title.HERO_WORDS * 4)
                self.assertEqual(names.variables - names.hero_continue, title.HERO_WORDS * 4)

    def test_counts_follow_the_c_sources(self):
        header = saved_game.HEADER
        self.assertEqual(title.LAYOUT_ENTRY_COUNT, define(title.SAVE_SOURCE, "SAVE_LAYOUT_ENTRIES"))
        self.assertEqual(title.COPIED_WORDS, define(title.SAVE_SOURCE, "SAVED_GAME_TEMPLATE_WORDS"))
        self.assertEqual(title.HERO_WORDS, define(title.SAVE_SOURCE, "HERO_TEMPLATE_WORDS"))
        self.assertEqual(title.HERO_WORDS * 4, define(header, "SAVED_CHARACTER_SIZE"))
        self.assertEqual(title.COPIED_WORDS * 4, define(header, "SAVED_GAME_DATA_SIZE"))
        self.assertEqual(saved_game.CHARACTER_SIZE, define(header, "SAVED_CHARACTER_SIZE"))
        for value, name in (
            (saved_game.ITEM_NAME_LENGTH, "FIELD_ITEM_NAME_LENGTH"),
            (saved_game.EQUIPMENT_SLOT_COUNT, "FIELD_EQUIPMENT_SLOT_COUNT"),
            (saved_game.STAT_COUNT, "FIELD_CHARACTER_STAT_COUNT"),
            (saved_game.WEAPON_CATEGORY_COUNT, "FIELD_WEAPON_CATEGORY_COUNT"),
            (saved_game.ABILITY_COUNT, "FIELD_ABILITY_COUNT"),
            (saved_game.ABILITY_WORD_COUNT, "FIELD_ABILITY_WORD_COUNT"),
            (saved_game.LAND_COUNT, "FIELD_LAND_COUNT"),
            (saved_game.PARTY_SIZE, "FIELD_PARTY_SIZE"),
            (saved_game.ITEM_COUNT, "FIELD_ITEM_COUNT"),
            (saved_game.ITEM_KIND_COUNT, "FIELD_ITEM_KIND_COUNT"),
            (saved_game.STAT_BASE_MASK, "FIELD_STAT_BASE_MASK"),
            (saved_game.STAT_EFFECTIVE_SHIFT, "FIELD_STAT_EFFECTIVE_SHIFT"),
        ):
            with self.subTest(name=name):
                self.assertEqual(value, define(header, name))
        self.assertEqual(len(saved_game.WEAPON_CATEGORIES), saved_game.WEAPON_CATEGORY_COUNT)
        self.assertEqual(saved_game.item_categories(), {0: "weapon", 1: "armor", 2: "instrument"})

    def test_loop_bounds_and_uploads_follow_the_code(self):
        save = title.SAVE_SOURCE.read_text(encoding="ascii")
        self.assertEqual(title.TEXTURE_COUNT, define(title.SAVE_SOURCE, "SAVE_LAYOUT_TEX_COUNT"))
        self.assertIn(f"next_index >= 0x{title.WEAPON_SLOT_COUNT:X}", save)
        self.assertIn(f"uv->u * {title.UV_UNIT_PIXELS}", save)
        menu = TITLE_SOURCE.read_text(encoding="ascii")
        self.assertIn(f"g_cursor_blink_u_offsets[(g_title_anim_frame >> 2) & {title.CURSOR_FRAME_COUNT - 1}]", menu)
        calls = re.findall(r"upload_tim\(\(void\*\)\(\(\(u8\*\)&g_title_menu_tim_table\) \+ g_title_menu_tim_table\[(\d)\]\), "
                           r"(0x[0-9A-F]+|\d+), (0x[0-9A-F]+|\d+), (0x[0-9A-F]+|\d+), (0x[0-9A-F]+|\d+)\);", menu)
        found = {title.MENU_TIM_NAMES[int(index) - 1]: tuple(int(value, 0) for value in values)
                 for index, *values in calls}
        expected = {name: (upload["image"]["x"], upload["image"]["y"], upload["palettes"]["x"], upload["palettes"]["y"])
                    for name, upload in title.MENU_UPLOADS.items()}
        self.assertEqual(found, expected)


class SavedGameLayoutTest(unittest.TestCase):
    """Compare the reader's offsets with the PS1 layout of include/saved_game.h."""

    @classmethod
    def setUpClass(cls):
        try:
            import clang.cindex as ci
        except ImportError as error:  # libclang is also what data2c needs
            raise unittest.SkipTest(f"libclang is not available: {error}")
        args = ["--target=mipsel-unknown-linux-gnu", "-std=gnu89", "-DVERSION_US", "-Wno-everything",
                f"-I{REPO_ROOT / 'include'}", f"-I{REPO_ROOT / 'include/sdk'}"]
        unit = ci.Index.create().parse("layout.c", args=args,
                                       unsaved_files=[("layout.c", '#include "saved_game.h"\n')])
        cls.types = {}
        for cursor in unit.cursor.walk_preorder():
            if cursor.kind == ci.CursorKind.TYPEDEF_DECL:
                cls.types[cursor.spelling] = cursor.underlying_typedef_type

    def check(self, type_name: str, offsets: dict[str, int], size: int):
        layout = self.types[type_name]
        self.assertEqual(layout.get_size(), size)
        for name, offset in offsets.items():
            with self.subTest(type=type_name, member=name):
                self.assertEqual(layout.get_offset(name), offset * 8)

    def test_item_record(self):
        self.check("FieldItemRecord", saved_game.ITEM_OFFSETS, saved_game.ITEM_SIZE)

    def test_character_record(self):
        self.check("FieldCharacterRecord", saved_game.CHARACTER_OFFSETS, saved_game.CHARACTER_SIZE)

    def test_saved_game_layout(self):
        self.check("SavedGameLayout", saved_game.LAYOUT_OFFSETS, saved_game.LAYOUT_SIZE)


if __name__ == "__main__":
    unittest.main()
