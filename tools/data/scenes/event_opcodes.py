"""Event command layouts from field_script_ops.c and field_script_commands.c.

Basic descriptor shifts are written explicitly: not every handler reads its
operand types in the same order. Pair and extended commands always consume
respectively two and four operands, even when their handlers ignore some.
"""

from typing import NamedTuple


class FixedOpcode(NamedTuple):
    name: str
    format: str
    fields: tuple[str, ...]


class TypedOpcode(NamedTuple):
    name: str
    fields: tuple[str, ...]
    shifts: tuple[int, ...]
    owners: tuple[str, ...] = ()


FIXED = {
    0x00: FixedOpcode("return", "", ()),
    0x01: FixedOpcode("jump", "h", ("delta",)),
    0x02: FixedOpcode("call", "h", ("delta",)),
    0x03: FixedOpcode("field_command", "B", ("command_id",)),
    0x04: FixedOpcode("branch_if_true", "h", ("delta",)),
    0x05: FixedOpcode("branch_if_false", "h", ("delta",)),
    0x06: FixedOpcode("nop", "", ()),
    0x07: FixedOpcode("copy_condition_flag", "", ()),
    0x0F: FixedOpcode("debug_command", "B", ("subcommand",)),
    0x10: FixedOpcode("call_event_variable", "H", ("script_ref",)),
    0x11: FixedOpcode("talk", "BHBBB", ("speaker", "string_index", "portrait", "window_flags", "layout")),
    0x12: FixedOpcode("wait_condition", "BB", ("selector", "argument")),
    0x13: FixedOpcode("wait_actor_variable", "BH", ("selector", "actor_ref")),
    0x14: FixedOpcode("wait_field_condition", "B", ("selector",)),
    0x15: FixedOpcode("wait_text_window", "B", ("window",)),
    0x16: FixedOpcode("wait_actor_idle", "B", ("actor",)),
    0x18: FixedOpcode("seek_scene", "H", ("scene_id",)),
    0x19: FixedOpcode("play_sound", "BB", ("sound_id", "pan")),
    0x1A: FixedOpcode("set_actor_control", "BB", ("actor", "mode")),
    0x1E: FixedOpcode("start_battle", "B", ("group",)),
    0x1F: FixedOpcode("suspend_battle", "", ()),
    0x27: FixedOpcode("remove_party_member", "B", ("member",)),
    0x29: FixedOpcode("wait_ring_menu", "", ()),
    0x2E: FixedOpcode("stalled_nop", "", ()),  # Handler does not advance PC.
    0x38: FixedOpcode("nop", "", ()),
}
TYPED = {
    0x0A: TypedOpcode("wait_frames", ("frames",), (0,)),
    0x28: TypedOpcode("open_ring_menu", ("menu_id", "excluded_mask", "cancel_index"), (6, 4, 2)),
    0x2A: TypedOpcode("unused_operands", ("unused_0", "unused_1", "unused_2", "unused_3"), (6, 4, 2, 0)),
    0x2B: TypedOpcode("format_number", ("window", "value", "digits"), (6, 4, 2)),
    0x2C: TypedOpcode("close_text_window", ("window",), (0,)),
    0x2D: TypedOpcode("face_actor", ("actor", "target"), (6, 4), ("actor", "target")),
    0x2F: TypedOpcode("set_actor_group", ("actor", "group"), (6, 4), ("actor",)),
    0x31: TypedOpcode("set_actor_position", ("actor", "x", "y", "z"), (6, 4, 2, 0), ("actor",)),
    0x32: TypedOpcode("open_text_window", ("speaker", "plane", "portrait", "layout"), (6, 4, 2, 0), ("speaker",)),
    0x33: TypedOpcode("show_text", ("window", "string_index", "options"), (6, 4, 2)),
    0x34: TypedOpcode("play_group_0_sound", ("sound_id", "pan", "unused"), (0, 2, 4)),
    0x35: TypedOpcode("targeted_animation", ("actor", "target", "resource_index"), (0, 2, 4), ("actor", "target")),
    0x37: TypedOpcode("open_shop", ("list_index", "price_scale"), (6, 4)),
}
# Pair/extended descriptor nibbles are low-first. Names identify handler arguments.
PAIR = {
    0x40: ("set_variable", ("destination", "value")),
    0x41: ("stop_tile_animation", ("index", "keyframe")),
    0x42: ("load_resource", ("resource_slot_id", "resource_base")),
    0x43: ("test_stored_position", ("mode", "actor")),
    0x44: ("misc_command", ("subcommand", "argument")),
    0x45: ("set_actor_animation", ("actor", "unused")),
    0x46: ("test_depth_overlap", ("first_actor", "second_actor")),
    0x47: ("join_guest", ("actor", "guest_id")),
    0x48: ("join_companion", ("actor", "source")),
    0x49: ("join_golem", ("actor", "logic_type")),
    0x4A: ("start_actor_script", ("actor", "entry")),
    0x4B: ("show_part", ("object_index", "part_index")),
    0x4C: ("hide_part", ("object_index", "part_index")),
    0x4D: ("set_actor_height", ("actor", "height")),
    0x4E: ("start_animation", ("actor", "resource")),
    0x4F: ("open_shop", ("list_index", "price_scale")),
    0x50: ("retire_actor", ("actor", "resource_index")),
    0x51: ("test_facing", ("first_actor", "second_actor")),
    0x52: ("copy_variable", ("destination", "source")),
    0x53: ("fade_second_song", ("volume", "frames")),
    0x54: ("play_song_section", ("unused_0", "unused_1")),
    0x55: ("fade_song", ("volume", "frames")),
    0x56: ("set_land_flag_bits_4_5", ("land", "value")),
    0x57: ("reset_actor", ("actor", "resource_entry_index")),
    0x58: ("play_sound", ("sound_id", "pan")),
    0x59: ("read_money", ("destination", "unused")),
    0x5A: ("read_actor_binding", ("destination", "actor")),
    **{opcode: ("nop", ("unused_0", "unused_1")) for opcode in range(0x5B, 0x60)},
}
EXTENDED = {
    0x80: ("diagnostic", ("status", "code", "value_0", "value_1")),
    0x81: ("set_actor_render_state", ("mode", "red", "green", "blue")),
    0x82: ("reload_actor", ("actor_key", "resource_entry_index", "resource_slot_id", "resource_base")),
    0x83: ("nop", ("unused_0", "unused_1", "unused_2", "unused_3")),
    0x84: ("change_battle_entry", ("actor", "animation", "builtin_animation", "sound")),
    0x85: ("change_scene", ("scene_id", "object_id", "audio", "spawn_id")),
    0x86: ("set_text_macro", ("slot", "resource_id", "entry_index", "character_limit")),
    0x87: ("actor_event", ("selector", "actor", "event_id", "argument")),
    0x88: ("game_over", ("image_resource_index", "music_resource_index", "audio_clip_index", "unused")),
    0x89: ("fade_sfx_volume", ("unused_0", "unused_1", "value", "value_2")),
    0x8A: ("targeted_animation", ("actor", "resource_index", "target", "unused")),
    0x8B: ("fade_color", ("red", "green", "blue", "timer")),
    0x8C: ("revive_actor", ("actor", "animation", "effect", "sound")),
    **{opcode: ("nop", ("unused_0", "unused_1", "unused_2", "unused_3")) for opcode in range(0x8D, 0x90)},
}
# Only these handler arguments replace 0xFF with the script owner's actor ID.
OWNER_FIELDS = {
    0x43: ("actor",), 0x45: ("actor",), 0x46: ("first_actor", "second_actor"),
    0x47: ("actor",), 0x48: ("actor",), 0x49: ("actor",), 0x4A: ("actor",),
    0x4D: ("actor",), 0x4E: ("actor",), 0x50: ("actor",),
    0x51: ("first_actor", "second_actor"), 0x57: ("actor",), 0x5A: ("actor",),
    0x84: ("actor",), 0x87: ("actor",), 0x8A: ("actor", "target"), 0x8C: ("actor",),
}
CALCULATIONS = ("add", "subtract", "multiply", "unsigned_divide", "unsigned_remainder", "bit_and",
                "bit_or", "bit_xor", "random", "unsigned_maximum", "unsigned_minimum", "unused")
FIELD_COMMANDS = (
    "clear_params", "actor_status", "script_party", "end_interaction", "set_fade", "test_area",
    "read_nibble_table", "get_part_position", "items", "query_records", "scroll_camera", "place_lands",
    "set_actor_position", "set_render_state", "script_pad_party", "spawn_monster", "get_actor_position",
)

# Subcommands of opcode 0x44, from its enum and switch in field_script_ops.c.
MISC_COMMANDS = {
    0x00: "return_to_title",
    0x01: "start_timed_panel",
    0x02: "set_game_flag",
    0x03: "stop_actor",
    0x04: "effect_animation_1",
    0x05: "effect_animation_0",
    0x06: "set_land_flag_04",
    0x07: "restart_tile_animation",
    0x08: "finish_tile_animation",
    0x09: "stop_second_song",
    0x0A: "open_encyclopedia",
    0x0B: "menu",
    0x0C: "screen_sequence",
    0x0D: "create_item",
    0x0E: "play_second_song",
    0x0F: "make_land_available",
    0x10: "leave_party",
    0x11: "stop_actor_script",
    0x12: "get_item_count",
    0x13: "receive_item",
    0x14: "consume_item",
    0x15: "get_land_state",
    0x16: "toggle_actor_hidden",
    0x17: "turn_actor",
    0x18: "add_companion",
    0x19: "release_companion",
    0x1A: "set_script_only",
    0x1B: "start_interaction",
    0x1C: "restart_palette_animation",
    0x1D: "stop_palette_animation",
    0x1E: "restart_tint_animation",
    0x1F: "stop_tint_animation",
    0x20: "enable_node",
    0x21: "disable_node",
    0x22: "add_template_item",
    0x23: "select_golem_cell",
    0x24: "publish_golem_cell",
    0x25: "raise_companion_intensity",
    0x26: "discard_item",
    0x27: "stop_non_script_actors",
    0x28: "actor_loop_stub",
    0x29: "unlock_encyclopedia",
    0x2A: "open_carda",
    0x2B: "set_variable",
    0x2C: "clear_script_only",
    0x2D: "get_companion_status",
    0x2E: "rename_companion",
    0x2F: "pixel_lookup_2f",
    0x32: "unknown_32",
    0x33: "disable_pair_indicators",
    0x34: "find_faced_item",
    0x35: "sequence_0",
    0x36: "sequence_1",
    0x37: "show_timed_text",
    0x38: "defeat_actor",
    0x39: "receive_money",
    0x3A: "spend_money",
    0x3B: "set_music_track_index",
    0x3C: "advance_weekday",
    0x3D: "hide_actor_panels",
    0x3E: "open_shop",
    0x3F: "cache_inventory_values",
    0x40: "stop_owner_actor",
    0x41: "apply_region_effects",
    0x42: "fade_out",
    0x43: "pixel_lookup_43",
    0x44: "set_duel_mode",
    0x45: "stop_song",
    0x46: "audio_f1",
    0x47: "reset_party_level",
    0x48: "apply_region_level_ups",
    0x49: "unknown_49",
    0x4A: "wait",
    0x4B: "set_gosub_result",
    0x4C: "set_money",
    0x4D: "open_carda_scene",
    0x4E: "new_game",
}
