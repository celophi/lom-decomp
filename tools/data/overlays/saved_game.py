"""Decode the saved-game records described in include/common/saved_game.h.

TITLE's game-state templates are SavedGameLayout images, its hero records are
FieldCharacterRecords and its starting weapons are FieldItemRecords. The
offsets below follow those C structs; tests/test_title_sources.py compares
them with the header through libclang. Readers return plain dictionaries for
YAML and leave any area they don't name to the raw bytes kept beside them.
"""

from __future__ import annotations

from functools import cache
from pathlib import Path
import re
import struct
from typing import Callable

REPO_ROOT = Path(__file__).resolve().parents[3]
HEADER = REPO_ROOT / "include/common/saved_game.h"

ITEM_SIZE = 0x40
ITEM_NAME_LENGTH = 0x14  # FIELD_ITEM_NAME_LENGTH
CHARACTER_SIZE = 0x250  # SAVED_CHARACTER_SIZE
CHARACTER_NAME_LENGTH = 24
EQUIPMENT_SLOT_COUNT = 4  # FIELD_EQUIPMENT_SLOT_COUNT
STAT_COUNT = 8  # FIELD_CHARACTER_STAT_COUNT
LAYOUT_SIZE = 0x3260  # sizeof(SavedGameLayout)
SUMMARY_NAME_LENGTH = 21
WEAPON_CATEGORY_COUNT = 11  # FIELD_WEAPON_CATEGORY_COUNT
ABILITY_COUNT = 88  # FIELD_ABILITY_COUNT
ABILITY_WORD_COUNT = 3  # FIELD_ABILITY_WORD_COUNT
LAND_COUNT = 64  # FIELD_LAND_COUNT
LAND_SIZE = 0xC
PARTY_SIZE = 3  # FIELD_PARTY_SIZE
ITEM_COUNT = 100  # FIELD_ITEM_COUNT
ITEM_KIND_COUNT = 256  # FIELD_ITEM_KIND_COUNT
LAYOUT_WORD_COUNT = 0x80  # SavedGameLayout::words
STAT_BASE_MASK = 0x1FF  # FIELD_STAT_BASE_MASK
STAT_EFFECTIVE_SHIFT = 9  # FIELD_STAT_EFFECTIVE_SHIFT

# Weapon categories in the order of the technique masks, proficiency slots and
# weapon item types. FIELD's code uses the same order (FIELD_WEAPON_FLAIL is 9).
WEAPON_CATEGORIES = (
    "knife", "sword", "axe", "two_handed_sword", "two_handed_axe", "hammer",
    "spear", "staff", "glove", "flail", "bow",
)

# FieldItemRecord members.
ITEM_OFFSETS = {
    "name": 0x00, "info": 0x14, "bonus_nibbles": 0x18, "stat_nibbles": 0x1C, "special_ids": 0x20,
    "derived": 0x24, "status_flags": 0x2C, "element_flags": 0x2D, "effect_index": 0x2E,
    "attributes": 0x30, "value": 0x34, "key": 0x38,
}

# FieldCharacterRecord members.
CHARACTER_OFFSETS = {
    "name": 0x00, "info": 0x18, "progress": 0x20, "hp": 0x24, "unk26": 0x26, "equipment_totals": 0x28,
    "stats": 0x30, "unk40": 0x40, "duel_wins": 0x44, "duel_losses": 0x46, "button_actions": 0x48,
    "equipment": 0x50, "unk150": 0x150,
}

# SavedGameLayout members.
LAYOUT_OFFSETS = {
    "summary_name": 0x00, "summary_slot_count": 0x17, "spawn": 0x18, "sound_bank_id": 0x1C,
    "secondary_music_id": 0x1E, "icon_palette": 0x1F, "track": 0x20, "scene_id": 0x24, "object_id": 0x26,
    "music_id": 0x27, "options": 0x28, "money": 0x2C, "play_time": 0x30, "technique_bits": 0x34,
    "ability_bits": 0x60, "ability_proficiency": 0x6C, "weapon_proficiency": 0xC4, "compatibility_tag": 0xCF,
    "identity": 0xD4, "guest_origin": 0xD8, "guest_loaded": 0xDE, "world_map_cell": 0xE0, "words": 0xE4,
    "control": 0x2E4, "flag_bits": 0x2E8, "lands": 0x2F0, "characters": 0x5F0, "items": 0xCE0,
    "item_counts": 0x25E0, "menu_slots": 0x26E4, "golem_count": 0x29D4, "joined_golem": 0x29D7,
    "joined_pet": 0x2EF0, "encyclopedia_bits": 0x30D4, "retry_count": 0x315C,
}

TextReader = Callable[[bytes], str]


@cache
def item_categories() -> dict[int, str]:
    """FIELD_ITEM_CATEGORY_* values from saved_game.h, as lowercase names."""
    source = HEADER.read_text(encoding="ascii")
    found = re.findall(r"FIELD_ITEM_CATEGORY_(\w+)\s*=\s*(\d+)", source)
    if not found:
        raise ValueError(f"{HEADER} has no FIELD_ITEM_CATEGORY_* values")
    return {int(value): name.lower() for name, value in found}


def nibbles(word: int) -> list[int]:
    return [(word >> shift) & 15 for shift in range(0, 32, 4)]


def text_field(raw: bytes, read_text: TextReader, two_byte_codes: frozenset[int]) -> dict[str, str]:
    """A fixed-size text field: decoded up to its terminator, with every stored byte."""
    end, position = len(raw), 0
    while position < len(raw):
        if raw[position] == 0:
            end = position
            break
        position += 2 if raw[position] in two_byte_codes else 1
    return {"text": read_text(raw[:end]), "bytes": raw.hex(" ")}


def item_record(raw: bytes, read_text: TextReader, two_byte_codes: frozenset[int],
                *, include_empty: bool = False) -> dict[str, object] | None:
    """Decode a FieldItemRecord; optionally include records marked free by an empty name."""
    if len(raw) != ITEM_SIZE:
        raise ValueError(f"item record is 0x{len(raw):X} bytes, not 0x{ITEM_SIZE:X}")
    if raw[0] == 0 and not include_empty:
        return None
    info, bonus, stat = struct.unpack_from("<3I", raw, ITEM_OFFSETS["info"])
    category = (info >> 8) & 3
    item_type = (info >> 10) & 0x3F
    status, element, effect = struct.unpack_from("<BBH", raw, ITEM_OFFSETS["status_flags"])
    value, first, second = struct.unpack_from("<3i", raw, ITEM_OFFSETS["value"])
    derived = raw[ITEM_OFFSETS["derived"] : ITEM_OFFSETS["status_flags"]]
    document = {
        "name": text_field(raw[:ITEM_NAME_LENGTH], read_text, two_byte_codes),
        "info": f"0x{info:08X}",
        "category": item_categories().get(category, category),
        "item_type": item_type,
    }
    if category == 0 and item_type < len(WEAPON_CATEGORIES):
        document["weapon_category"] = WEAPON_CATEGORIES[item_type]
    document.update({
        "material": (info >> 16) & 0x3F,
        "info_low_byte": info & 0xFF,
        "info_high_bits": info >> 22,
        "bonus_nibbles": nibbles(bonus),
        "stat_nibbles": nibbles(stat),
        "special_ids": list(raw[0x20:0x24]),
        "derived": derived.hex(" "),
    })
    if category == 0:
        document["weapon_power"] = struct.unpack_from("<H", derived)[0]
    document.update({
        "status_flags": f"0x{status:02X}", "element_flags": f"0x{element:02X}", "effect_index": effect,
        "attributes": list(raw[0x30:0x34]), "value": value, "key": [first, second],
    })
    return document


def item_list(raw: bytes, count: int, read_text: TextReader, codes: frozenset[int]) -> list[dict[str, object]]:
    """The records in use, each with its index; free records are left out."""
    items = []
    for index in range(count):
        record = item_record(raw[index * ITEM_SIZE : (index + 1) * ITEM_SIZE], read_text, codes)
        if record is not None:
            items.append({"index": index, **record})
    return items


def character_record(raw: bytes, read_text: TextReader, two_byte_codes: frozenset[int]) -> dict[str, object]:
    """Decode a FieldCharacterRecord; its bytes stay in the raw copy."""
    if len(raw) != CHARACTER_SIZE:
        raise ValueError(f"character record is 0x{len(raw):X} bytes, not 0x{CHARACTER_SIZE:X}")
    info = raw[0x18:0x20]
    progress, hp, unk26 = struct.unpack_from("<IHH", raw, CHARACTER_OFFSETS["progress"])
    stats = struct.unpack_from(f"<{STAT_COUNT}H", raw, CHARACTER_OFFSETS["stats"])
    wins, losses = struct.unpack_from("<HH", raw, CHARACTER_OFFSETS["duel_wins"])
    equipment = raw[CHARACTER_OFFSETS["equipment"] : CHARACTER_OFFSETS["unk150"]]
    extra = raw[CHARACTER_OFFSETS["unk150"] :]
    return {
        "name": text_field(raw[:CHARACTER_NAME_LENGTH], read_text, two_byte_codes),
        "character_type": info[0] & 0x7F,
        "pad_controlled": bool(info[0] & 0x80),
        "info_byte_1": info[1],
        "commands": list(info[2:4]),
        "skills": list(info[4:8]),
        "level": progress & 0xFF,
        "experience": progress >> 8,
        "hp": hp,
        "unk26": unk26,
        "equipment_totals": list(struct.unpack_from("<4H", raw, CHARACTER_OFFSETS["equipment_totals"])),
        "stats": [{"base": value & STAT_BASE_MASK, "effective": value >> STAT_EFFECTIVE_SHIFT} for value in stats],
        "unk40": list(raw[0x40:0x44]),
        "duel_wins": wins,
        "duel_losses": losses,
        "button_actions": list(raw[0x48:0x50]),
        "equipment": [item_record(equipment[slot * ITEM_SIZE : (slot + 1) * ITEM_SIZE], read_text, two_byte_codes)
                      for slot in range(EQUIPMENT_SLOT_COUNT)],
        "unk150": item_list(extra, len(extra) // ITEM_SIZE, read_text, two_byte_codes),
    }


def layout(raw: bytes, read_text: TextReader, two_byte_codes: frozenset[int]) -> dict[str, object]:
    """Decode the named SavedGameLayout fields of @p raw (at least LAYOUT_SIZE bytes)."""
    if len(raw) < LAYOUT_SIZE:
        raise ValueError(f"saved-game layout is 0x{len(raw):X} bytes, shorter than 0x{LAYOUT_SIZE:X}")
    at = LAYOUT_OFFSETS

    def word(name: str, fmt: str = "<I") -> int:
        return struct.unpack_from(fmt, raw, at[name])[0]

    spawn, track, options = word("spawn"), word("track"), word("options")
    game_id, save_id = struct.unpack_from("<HH", raw, at["identity"])
    guest_game, guest_save = struct.unpack_from("<HH", raw, at["guest_origin"])
    control = word("control")
    techniques = struct.unpack_from(f"<{WEAPON_CATEGORY_COUNT}I", raw, at["technique_bits"])
    proficiency = raw[at["weapon_proficiency"] : at["weapon_proficiency"] + WEAPON_CATEGORY_COUNT]
    words = struct.unpack_from(f"<{LAYOUT_WORD_COUNT}i", raw, at["words"])
    lands = []
    for index in range(LAND_COUNT):
        land = raw[at["lands"] + index * LAND_SIZE : at["lands"] + (index + 1) * LAND_SIZE]
        if any(land):
            lands.append({"index": index, "flags": f"0x{land[0]:02X}", "x": land[1] & 15, "z": land[1] >> 4,
                          "unk2": land[2], "count": land[3], "levels": list(land[4:])})
    characters = []
    for slot in range(PARTY_SIZE):
        start = at["characters"] + slot * CHARACTER_SIZE
        characters.append({"slot": slot, **character_record(raw[start : start + CHARACTER_SIZE], read_text, two_byte_codes)})
    counts = raw[at["item_counts"] : at["item_counts"] + ITEM_KIND_COUNT]
    return {
        "summary_name": text_field(raw[:SUMMARY_NAME_LENGTH], read_text, two_byte_codes),
        "summary_slot_count": raw[at["summary_slot_count"]],
        "spawn": {"id": spawn & 0x1FFFFFF, "party_icon_0": spawn >> 25},
        "sound_bank_id": word("sound_bank_id", "<h"),
        "secondary_music_id": word("secondary_music_id", "<b"),
        "icon_palette": raw[at["icon_palette"]],
        "track": {"music_track": track & 0x3FFFF, "party_icon_1": (track >> 18) & 0x7F, "party_icon_2": track >> 25},
        "scene_id": word("scene_id", "<H"),
        "object_id": raw[at["object_id"]],
        "music_id": raw[at["music_id"]],
        "options": {"word": f"0x{options:08X}", "vibration": bool(options & 1), "mono_sound": bool(options & 2),
                    "flag_2": bool(options & 4), "flag_3": bool(options & 8)},
        "money": word("money"),
        "play_time": word("play_time", "<i"),
        "technique_bits": {name: f"0x{value:08X}" for name, value in zip(WEAPON_CATEGORIES, techniques)},
        "ability_bits": [f"0x{value:08X}" for value in struct.unpack_from(f"<{ABILITY_WORD_COUNT}I", raw, at["ability_bits"])],
        "ability_proficiency": list(raw[at["ability_proficiency"] : at["ability_proficiency"] + ABILITY_COUNT]),
        "weapon_proficiency": dict(zip(WEAPON_CATEGORIES, proficiency)),
        "compatibility_tag": raw[at["compatibility_tag"]],
        "identity": {"game_id": game_id, "save_id": save_id},
        "guest_origin": {"game_id": guest_game, "save_id": guest_save},
        "guest_loaded": word("guest_loaded", "<H"),
        "world_map_cell": word("world_map_cell", "<i"),
        "words": {f"0x{index:02X}": value for index, value in enumerate(words) if value},
        "control": {"placed_land_count": control & 0xFF, "hero_level": (control >> 8) & 0xFF,
                    "weekday": (control >> 16) & 0x7F, "high_bits": control >> 23},
        "flag_bits": [f"0x{value:08X}" for value in struct.unpack_from("<2I", raw, at["flag_bits"])],
        "lands": lands,
        "characters": characters,
        "items": item_list(raw[at["items"] : at["item_counts"]], ITEM_COUNT, read_text, two_byte_codes),
        "item_counts": {index: count for index, count in enumerate(counts) if count},
        "golem_count": raw[at["golem_count"]] & 0xF,
        "joined_golem": struct.unpack_from("<b", raw, at["joined_golem"])[0],
        "joined_pet": word("joined_pet", "<i"),
        "encyclopedia_bits": raw[at["encyclopedia_bits"] : at["encyclopedia_bits"] + 0x80].hex(" "),
        "retry_count": word("retry_count", "<i"),
    }
