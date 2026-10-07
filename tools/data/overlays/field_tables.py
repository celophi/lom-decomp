"""Layouts of FIELD's small tables, as used by the C sources.

Counts without a fixed C array bound cover the stored run of values up to the
next resource, including alignment bytes. The YAML keeps those values too.
Larger formats with their own headers are read in field.py.
"""

from dataclasses import dataclass


@dataclass(frozen=True)
class TableSpec:
    """One array: its symbol, little-endian record format and stored row count."""

    symbol: str
    format: str
    count: int
    fields: tuple[str, ...] = ()
    resolve_symbols: bool = False


# HUD, actor animation and input tables (field_actor_hud_effects.c,
# field_actor_behavior.c, field_actor_input_map_init.c and field_scene_transition.c).
TABLES = (
    TableSpec("g_field_pixel_lookup_tables", "H", 32),
    TableSpec("g_field_hud_hp_colors", "I", 5),
    TableSpec("g_field_hud_status_colors", "4B", 2, ("r", "g", "b", "pad")),
    TableSpec("g_field_hud_effect_colors", "4B", 2, ("r", "g", "b", "pad")),
    TableSpec("g_field_hud_full_status_colors", "4B", 2, ("r", "g", "b", "pad")),
    TableSpec("g_field_hud_companion_status_colors", "4B", 2, ("r", "g", "b", "pad")),
    TableSpec("g_field_object_flag_handlers", "I", 16, resolve_symbols=True),
    TableSpec("g_field_party_hud_order", "i", 3),
    TableSpec("g_field_hud_shake_offsets", "h", 8),
    TableSpec("g_field_weapon_chain_limits", "B", 11),
    TableSpec("g_field_actor_turn_animations", "B", 48),
    TableSpec("g_field_direction_animation_modes", "i", 8),
    TableSpec("g_field_actor_walk_animations", "i", 8),
    TableSpec("g_field_actor_diagonal_walk_animations", "i", 12),
    TableSpec("g_field_action_animation_parameters", "B", 164),
    TableSpec("g_field_action_sound_ids", "B", 84),
    TableSpec("g_field_follower_animation_map", "B", 16),
    TableSpec("g_field_weapon_action_slots", "B", 5),
    TableSpec("g_field_weapon_action_default_params", "B", 5),
    TableSpec("g_field_weapon_action_blunt_params", "B", 5),
    TableSpec("g_field_weapon_action_bow_params", "B", 5),
    TableSpec("g_field_binding_buttons", "B", 8),
    TableSpec("g_field_action_binding_codes", "B", 8),
    TableSpec("g_field_instrument_icons", "B", 8),
    TableSpec("g_field_direction_offsets", "hh", 8, ("x", "z")),
    TableSpec("g_field_chest_geometry", "B", 60),
    TableSpec("g_field_party_palettes", "H", 6),
    # Target selection, command history, pair indicators and audio.
    TableSpec("g_field_target_filters", "I", 7, resolve_symbols=True),
    TableSpec("g_field_action_command_codes", "B", 8),
    TableSpec("g_field_hint_button_map", "B", 8),
    TableSpec("g_field_command_patterns", "8B", 6),
    TableSpec("g_field_command_pattern_ids", "B", 6),
    TableSpec("g_field_pair_indicator_corners", "hh", 16, ("x", "y")),
    TableSpec("g_field_ribbon_frame_flags", "B", 12),
    TableSpec("g_field_ribbon_uv_corners", "BB", 8, ("u", "v")),
    TableSpec("g_field_sfx_set_uses_slots", "B", 44),
    # Window layouts and battle status tables.
    TableSpec("g_field_text_window_layouts", "4H", 64, ("x", "y", "width", "height")),
    TableSpec("g_field_talk_plane_masks", "B", 16),
    TableSpec("g_field_timed_panel_modes", "B", 100),
    TableSpec("g_field_monster_level_by_rank", "B", 64),
    TableSpec("g_field_status_immunity_masks", "B", 16),
    TableSpec("g_field_status_duration_stats", "B", 16),
    TableSpec("g_field_element_level_by_land_level", "B", 8),
    TableSpec("g_field_stat_change_signals", "B", 8),
    TableSpec("g_field_equipment_status_flags", "H", 32),
    TableSpec("g_field_action_handlers", "I", 8, resolve_symbols=True),
    TableSpec("g_field_element_resist_slots", "B", 8),
    TableSpec("g_field_on_hit_statuses", "BB", 16, ("chance", "duration")),
    TableSpec("g_field_weapon_type_conflicts", "B", 11),
    TableSpec("g_field_armor_type_conflicts", "B", 11),
    # Script dispatch, rewards, growth and menu dispatch.
    TableSpec("g_field_script_op_table", "I", 64, resolve_symbols=True),
    TableSpec("g_field_script_pair_op_table", "I", 32, resolve_symbols=True),
    TableSpec("g_field_script_ext_op_table", "I", 16, resolve_symbols=True),
    TableSpec("g_field_script_var_widths", "B", 8),
    TableSpec("g_field_script_commands", "I", 17, resolve_symbols=True),
    TableSpec("g_field_record_queries", "I", 1, resolve_symbols=True),
    TableSpec("g_field_script_calc_ops", "I", 12, resolve_symbols=True),
    TableSpec("g_field_stat_modifier_limits", "BB", 8, ("minimum", "maximum")),
    TableSpec("g_field_drop_handlers", "I", 4, resolve_symbols=True),
    TableSpec("g_field_drop_slots_by_level", "B", 8),
    TableSpec("g_field_item_type_stat_growth", "I", 16),
    TableSpec("g_field_level_experience", "I", 32),
    TableSpec("g_field_land_distance_items", "B", 32),
    TableSpec("g_field_menu_ops", "I", 96, resolve_symbols=True),
    TableSpec("g_golem_logic_block_recipe_table", "I", 64, resolve_symbols=True),
    TableSpec("g_golem_logic_block_level_scale", "B", 64),
    TableSpec("g_golem_logic_block_class", "i", 58),
    TableSpec("g_golem_logic_block_icons", "HH", 58, ("clut", "reserved")),
)
