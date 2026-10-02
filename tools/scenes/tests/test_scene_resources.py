"""Synthetic battle tables, including offsets, drop keys and damaged records."""

import struct
import unittest

from tools.scenes.scene_resources import read_battle_resource, read_resource_directory


def battle_resource():
    """Build one monster with all four drop handlers and one reward item."""
    monster = bytearray(0x5C)
    monster[:12] = b"Test monster"
    monster[0x18] = 7
    struct.pack_into("<4H", monster, 0x1C, 25, 0xFFFF, 3, 4)
    monster[0x24:0x28] = bytes([5, 6, 7, 8])
    monster[0x2C:0x30] = bytes([9, 10, 11, 12])
    monster[0x40:0x50] = bytes([0, 0x32, 1, 64, 2, 0x96, 3, 2, 255, 0, 0, 0, 0, 0, 0, 0])
    struct.pack_into("<I", monster, 0x50, 1)
    monster[0x54:] = bytes(range(8))
    templates = struct.pack("<II", 1, 8) + monster
    item = bytearray(64)
    item[:9] = b"Test item"
    rewards = struct.pack("<HHI", 0xAAAA, 1, 7 * 16 + 2) + item
    return struct.pack("<III", 1, 12, 12 + len(templates)) + templates + rewards


class SceneResourceTests(unittest.TestCase):
    def test_shared_unsorted_and_empty_directory_entries(self):
        data = struct.pack("<7I", 16, 24, 16, 28, 9, 0, 12)
        offsets, resources = read_resource_directory(data)
        self.assertEqual(offsets, [16, 24, 16, 28])
        self.assertEqual(resources[0].slots, (0, 2))
        self.assertEqual(resources[1].document(0x100)["resource_id"], 12)
        self.assertEqual(resources[1].document(0x100)["offset"], "0x118")
        self.assertEqual(resources[2].data, b"")
        self.assertIsNone(resources[2].document(0)["resource_id"])

    def test_rejects_directory_offsets_outside_section(self):
        for data in (b"\x04", bytes(4), struct.pack("<II", 8, 4), struct.pack("<II", 8, 12)):
            with self.subTest(data=data):
                with self.assertRaises(ValueError):
                    read_resource_directory(data)

    def test_monster_stats_and_drop_meanings(self):
        result = read_battle_resource(battle_resource(), 0x100)
        monster = result["monsters"][0]
        self.assertEqual(monster["offset"], "0x114")
        self.assertEqual(monster["name_ascii"], "Test monster")
        self.assertEqual(monster["hp_base"], 25)
        self.assertTrue(monster["hp_uses_stat_4_curve"])
        self.assertEqual(monster["equipment_stats"][1], {"base": 7, "growth": 8})
        self.assertEqual(monster["stats"][1], {"base": 11, "growth": 12})
        self.assertEqual(monster["drops"][0]["experience_pickups"], 3)
        self.assertEqual(monster["drops"][0]["money_pickups"], 2)
        self.assertEqual(monster["drops"][1]["half_restore_chance_out_of_256"], 64)
        self.assertEqual(monster["drops"][2]["item_id"], 0x96)
        self.assertEqual(monster["drops"][3]["reward_key"], 114)
        self.assertEqual(monster["drops"][3]["reward_indices"], [0])
        self.assertEqual(monster["drops"][4]["kind"], "no_drop")
        self.assertEqual(monster["actions"][0]["bytes"], "00 01 02 03 04 05 06 07")
        self.assertEqual(monster["actions"][0]["handler"], 3)
        self.assertEqual(result["rewards"][0]["name_ascii"], "Test item")
        self.assertEqual(len(bytes.fromhex(result["rewards"][0]["item_bytes"])), 64)

    def test_non_ascii_names_and_missing_rewards_are_preserved(self):
        data = bytearray(battle_resource())
        data[20] = 0x81
        data[20 + 0x47] = 3
        result = read_battle_resource(data, 0)
        monster = result["monsters"][0]
        self.assertIsNone(monster["name_ascii"])
        self.assertTrue(monster["name_bytes"].startswith("81"))
        self.assertEqual(monster["drops"][3]["reward_indices"], [])

    def test_bad_battle_offsets_counts_and_actions(self):
        original = battle_resource()
        for offset, value in ((4, 8), (8, len(original) + 4), (12, 999), (16, 4), (20 + 0x50, 999)):
            with self.subTest(offset=offset):
                data = bytearray(original)
                struct.pack_into("<I", data, offset, value)
                with self.assertRaises(ValueError):
                    read_battle_resource(data, 0)
        with self.assertRaisesRegex(ValueError, "reward items"):
            read_battle_resource(original[:-1], 0)


if __name__ == "__main__":
    unittest.main()
