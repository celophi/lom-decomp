# ============================================================================
# Overlay registry and per-source toolchain routing
# ============================================================================

# Register an overlay by adding its lowercase directory name to OVERLAYS.
# Every direct .c file under src/overlays/<name>/ must appear in exactly one
# toolchain configuration:
#
#   overlay_<name>_gcc_272_cdk_g0_srcs
#   overlay_<name>_gcc_272_cdk_g0_nosched_srcs
#   overlay_<name>_gcc_272_cdk_g0_noexpand_srcs
#   overlay_<name>_gcc_272_gnu_g0_srcs
#   overlay_<name>_gcc_280_g0_srcs
#   overlay_<name>_gcc_280_g0_builtin_srcs
#   overlay_<name>_gcc_280_g4_srcs
#   overlay_<name>_gcc_280_g4_noexpand_srcs
#
# overlays.mk rejects missing, unknown, or multiply routed sources. If a linker
# script expects a standalone assets/<name>.o, define:
#
#   overlay_<name>_asset_src := assets/<name>.bin
#
# Splat databin assets referenced through .incbin do not use this setting.

OVERLAYS += addhero
overlay_addhero_gcc_272_cdk_g0_srcs := \
	src/overlays/addhero/overlay_header.c \
	src/overlays/addhero/addhero.c \
	src/overlays/addhero/addhero_card_sequence.c \
	src/overlays/addhero/addhero_card.c \
	src/overlays/addhero/addhero_glyph.c

OVERLAYS += carda
overlay_carda_gcc_272_cdk_g0_srcs := \
	src/overlays/carda/overlay_header.c \
	src/overlays/carda/func_80140918.c \
	src/overlays/carda/func_80140BAC.c \
	src/overlays/carda/func_80141250.c \
	src/overlays/carda/carda_build_ui_elements.c \
	src/overlays/carda/carda_core_state.c \
	src/overlays/carda/carda_terminate_multibyte_text.c \
	src/overlays/carda/carda_string_utils.c \
	src/overlays/carda/carda_checksum.c \
	src/overlays/carda/func_8014344C.c \
	src/overlays/carda/func_8014366C.c \
	src/overlays/carda/carda_progress_bar.c \
	src/overlays/carda/carda_load_prompts.c \
	src/overlays/carda/func_80143DF4.c \
	src/overlays/carda/func_80143F90.c \
	src/overlays/carda/func_80144050.c \
	src/overlays/carda/func_801443F0.c \
	src/overlays/carda/func_801447DC.c \
	src/overlays/carda/func_80144A24.c \
	src/overlays/carda/carda_choice_prompt.c \
	src/overlays/carda/carda_choice_cancel.c \
	src/overlays/carda/carda_restore_record.c \
	src/overlays/carda/func_801466F8.c \
	src/overlays/carda/func_80146794.c \
	src/overlays/carda/func_80146CA4.c \
	src/overlays/carda/func_80146EDC.c \
	src/overlays/carda/carda_choice_init.c \
	src/overlays/carda/carda_hex_parse.c \
	src/overlays/carda/carda_panel_tiles.c \
	src/overlays/carda/carda_mode_glyph.c \
	src/overlays/carda/carda_selected_entry_details.c \
	src/overlays/carda/carda_claim_element.c \
	src/overlays/carda/carda_update_and_draw_elements.c \
	src/overlays/carda/carda_format_slot.c \
	src/overlays/carda/carda_stream_reset.c \
	src/overlays/carda/carda_init_stream_handles.c \
	src/overlays/carda/carda_shutdown_handles.c \
	src/overlays/carda/carda_begin_entry_scan.c \
	src/overlays/carda/carda_commit_selected_entry.c \
	src/overlays/carda/carda_handles.c \
	src/overlays/carda/carda_draw_signed_decimal.c \
	src/overlays/carda/carda_text_render.c \
	src/overlays/carda/carda_cache_table.c \
	src/overlays/carda/carda_expand_text_glyph_codes.c \
	src/overlays/carda/carda_state_step.c \
	src/overlays/carda/carda_update_state.c \
	src/overlays/carda/carda_reset_menu_state.c \
	src/overlays/carda/carda_packet_helpers.c \
	src/overlays/carda/carda_check_slot_flag.c \
	src/overlays/carda/carda_validate_resource.c \
	src/overlays/carda/carda_format_decimal.c \
	src/overlays/carda/carda_nibble_pair.c \
	src/overlays/carda/carda_parse_hex_suffix_byte.c \
	src/overlays/carda/func_80147588.c \
	src/overlays/carda/func_801477CC.c \
	src/overlays/carda/carda_reset_entry_ranks.c \
	src/overlays/carda/carda_known_entry_type.c \
	src/overlays/carda/carda_fixed_prompts.c \
	src/overlays/carda/func_80147DCC.c \
	src/overlays/carda/func_8014A1C4.c \
	src/overlays/carda/carda_format_hex.c \
	src/overlays/carda/carda_glyph_builder.c \
	src/overlays/carda/carda_header_label.c \
	src/overlays/carda/func_80147100.c \
	src/overlays/carda/func_80149A4C.c \
	src/overlays/carda/func_80142E10.c \
	src/overlays/carda/func_80145050.c \
	src/overlays/carda/func_80147F4C.c
overlay_carda_gcc_280_g0_srcs := \
	src/overlays/carda/carda_scan_hex_digits.c

OVERLAYS += checkps
overlay_checkps_gcc_272_cdk_g0_srcs := \
	src/overlays/checkps/overlay_header.c \
	src/overlays/checkps/init.c \
	src/overlays/checkps/font.c
overlay_checkps_gcc_280_g0_srcs := src/overlays/checkps/kanji.c
overlay_checkps_gcc_272_gnu_g0_srcs := \
	src/overlays/checkps/pattern.c \
	src/overlays/checkps/cdrom.c \
	src/overlays/checkps/cdrom_data.c
# Preserve GCC/local switch labels for objdiff.  cdrom.c's only compiler-emitted
# .data is the control-flow anchor array, so discard it and recreate the target
# object's empty .data section without contributing any linked bytes.
overlay_checkps_gcc_272_gnu_as_extra_flags_cdrom := -L
overlay_checkps_gcc_272_gnu_objcopy_flags_cdrom := --remove-section=.data --remove-section=.text --rename-section=.text.cdrom=.text --add-section=.data=/dev/null --set-section-flags=.data=alloc,data
overlay_checkps_target_as_extra_flags_cdrom := -L

OVERLAYS += cload
overlay_cload_gcc_272_cdk_g0_srcs := \
	src/overlays/cload/overlay_header.c \
	src/overlays/cload/cload.c

OVERLAYS += field
# Sources follow FIELD.BIN.yaml address order within each compiler configuration.
overlay_field_gcc_272_gnu_g0_srcs :=

overlay_field_gcc_272_cdk_g0_srcs := \
	src/overlays/field/overlay_header.c \
	src/overlays/field/field_frame_commands.c \
	src/overlays/field/field_draw_state.c \
	src/overlays/field/field_fade.c \
	src/overlays/field/field_actor_runtime.c \
	src/overlays/field/field_effect_update.c \
	src/overlays/field/field_effect_dispatch.c \
	src/overlays/field/field_effect_frames.c \
	src/overlays/field/field_effect_primitives.c \
	src/overlays/field/field_effect_transform.c \
	src/overlays/field/field_effect_geometry.c \
	src/overlays/field/field_mesh_render.c \
	src/overlays/field/field_mesh_transform.c \
	src/overlays/field/field_mesh_part_animation.c \
	src/overlays/field/field_actor_slot_resources.c \
	src/overlays/field/field_actor_hud.c \
	src/overlays/field/field_actor_control_effects.c \
	src/overlays/field/field_actor_key_ops.c \
	src/overlays/field/field_actor_script_dispatch.c \
	src/overlays/field/field_actor_action_commands.c \
	src/overlays/field/field_actor_animation_commands.c \
	src/overlays/field/field_actor_reactions.c \
	src/overlays/field/field_actor_routes.c \
	src/overlays/field/field_actor_behavior.c \
	src/overlays/field/field_actor_input_map_init.c \
	src/overlays/field/field_actor_input_actions.c \
	src/overlays/field/field_actor_camera.c \
	src/overlays/field/field_actor_idle_ops.c \
	src/overlays/field/field_actor_motion.c \
	src/overlays/field/field_actor_movement_states.c \
	src/overlays/field/field_actor_action_runtime.c \
	src/overlays/field/field_actor_displacement.c \
	src/overlays/field/field_actor_resource_states.c \
	src/overlays/field/field_actor_animation_resume.c \
	src/overlays/field/field_actor_sequence_runtime.c \
	src/overlays/field/field_actor_transition_reset.c \
	src/overlays/field/field_contact_geometry.c \
	src/overlays/field/field_actor_resource_unpack.c \
	src/overlays/field/field_scene_transition.c \
	src/overlays/field/field_text_window_api.c \
	src/overlays/field/field_block_allocator.c \
	src/overlays/field/field_actor_target_queries.c \
	src/overlays/field/field_actor_movement_modes.c \
	src/overlays/field/field_actor_effects.c \
	src/overlays/field/field_path_interpolation.c \
	src/overlays/field/field_command_history.c \
	src/overlays/field/field_pair_indicators.c \
	src/overlays/field/field_audio_runtime.c \
	src/overlays/field/field_ring_selection.c \
	src/overlays/field/field_timed_panel.c \
	src/overlays/field/field_actor_text_queue.c \
	src/overlays/field/field_ability_progression.c \
	src/overlays/field/field_dialog_screens.c \
	src/overlays/field/field_immediate_text.c \
	src/overlays/field/field_input_text_session.c \
	src/overlays/field/field_modal_runtime.c \
	src/overlays/field/field_modal_stream_start.c \
	src/overlays/field/field_numeric_sprites.c \
	src/overlays/field/field_pair_rule_lookup.c \
	src/overlays/field/field_menu_windows.c \
	src/overlays/field/field_coordinate_icon.c \
	src/overlays/field/field_choice_labels.c \
	src/overlays/field/field_item_selection.c \
	src/overlays/field/field_text_session.c \
	src/overlays/field/field_text_quad.c \
	src/overlays/field/field_card_clock.c \
	src/overlays/field/field_character_name_flags.c \
	src/overlays/field/field_resource_load.c

overlay_field_gcc_272_cdk_g0_nosched_srcs := \
	src/overlays/field/field_subsystem_init.c

overlay_field_gcc_272_cdk_g0_nostrength_srcs := \
	src/overlays/field/field_actor_action_defaults.c

overlay_field_gcc_272_cdk_g0_noexpand_srcs :=

overlay_field_gcc_280_g0_builtin_srcs := \
	src/overlays/field/field_select_distance_bucket.c

overlay_field_gcc_280_g0_srcs := \
	src/overlays/field/field_interaction_start.c \
	src/overlays/field/field_event_dispatch.c \
	src/overlays/field/field_state_ops.c \
	src/overlays/field/field_party_setup.c \
	src/overlays/field/field_actor_templates.c \
	src/overlays/field/field_actor_lifecycle.c \
	src/overlays/field/field_status_ticks.c \
	src/overlays/field/field_action_descriptors.c \
	src/overlays/field/field_action_setup.c \
	src/overlays/field/field_progression_ops.c \
	src/overlays/field/field_action_modifiers.c \
	src/overlays/field/field_record_stat_ops.c \
	src/overlays/field/field_script_flow_ops.c \
	src/overlays/field/field_script_primary_ops.c \
	src/overlays/field/field_script_pair_ops.c \
	src/overlays/field/field_script_extended_ops.c \
	src/overlays/field/field_script_operands.c \
	src/overlays/field/field_script_commands.c \
	src/overlays/field/field_script_arith_ops.c \
	src/overlays/field/field_record_setup_ops.c \
	src/overlays/field/field_slot_pool_ops.c \
	src/overlays/field/field_generated_record_ops.c \
	src/overlays/field/field_record_effect_ops.c \
	src/overlays/field/field_reward_command_ops.c \
	src/overlays/field/field_record_growth_ops.c \
	src/overlays/field/field_record_lookup_ops.c \
	src/overlays/field/field_record_buffer_ops.c \
	src/overlays/field/field_record_position_queries.c \
	src/overlays/field/field_stat_counter_ops.c \
	src/overlays/field/field_saved_slot_ops.c \
	src/overlays/field/field_actor_record_ops.c \
	src/overlays/field/field_resource_table_ops.c \
	src/overlays/field/field_active_record_ops.c \
	src/overlays/field/field_record_table_ops.c \
	src/overlays/field/field_group_layout_ops.c \
	src/overlays/field/field_group_stat_transfer.c \
	src/overlays/field/field_group_derived_stats.c \
	src/overlays/field/field_menu_group_ops.c \
	src/overlays/field/field_menu_object_ops.c \
	src/overlays/field/field_menu_slot_ops.c \
	src/overlays/field/field_gosub_history_ops.c \
	src/overlays/field/field_menu_action_runtime.c \
	src/overlays/field/field_menu_record_setup.c \
	src/overlays/field/field_menu_record_transfer.c \
	src/overlays/field/field_menu_record_audio_ops.c \
	src/overlays/field/field_menu_record_state.c \
	src/overlays/field/field_menu_attribute_ops.c \
	src/overlays/field/field_menu_record_ops.c \
	src/overlays/field/field_layout_slot_state.c \
	src/overlays/field/field_equipment_combination_rules.c \
	src/overlays/field/field_golem_logic_blocks.c

overlay_field_gcc_280_g4_srcs :=

overlay_field_gcc_280_g4_noexpand_srcs := \
	src/overlays/field/field_scene_load.c \
	src/overlays/field/field_scene_build.c \
	src/overlays/field/field_render.c \
	src/overlays/field/field_animation.c \
	src/overlays/field/field_scene_control.c \
	src/overlays/field/field_collision.c \
	src/overlays/field/field_text.c

OVERLAYS += gname
overlay_gname_gcc_272_cdk_g0_srcs := src/overlays/gname/overlay_header.c src/overlays/gname/gname.c

OVERLAYS += golem
overlay_golem_gcc_272_cdk_g0_srcs := src/overlays/golem/overlay_header.c src/overlays/golem/golem.c

OVERLAYS += gosub
overlay_gosub_gcc_272_cdk_g0_srcs := \
	src/overlays/gosub/overlay_header.c \
	src/overlays/gosub/gosub.c \
	src/overlays/gosub/gosub_elements.c \
	src/overlays/gosub/gosub_lists.c \
	src/overlays/gosub/gosub_selection.c \
	src/overlays/gosub/gosub_runtime.c \
	src/overlays/gosub/gosub_render.c \
	src/overlays/gosub/gosub_dialogs.c \
	src/overlays/gosub/gosub_helpers.c

OVERLAYS += gover
overlay_gover_gcc_272_cdk_g0_srcs := src/overlays/gover/overlay_header.c src/overlays/gover/gover.c

OVERLAYS += menu
overlay_menu_gcc_272_cdk_g0_srcs := \
	src/overlays/menu/overlay_header.c \
	src/overlays/menu/menu.c \
	src/overlays/menu/menu_screens.c \
	src/overlays/menu/menu_lists.c \
	src/overlays/menu/menu_actions.c \
	src/overlays/menu/menu_card.c

OVERLAYS += movie
overlay_movie_gcc_280_g4_srcs := \
	src/overlays/movie/movie.c \
	src/overlays/movie/movie_stream.c
# The header word is a plain -G0 const so it stays in .rodata.
overlay_movie_gcc_272_cdk_g0_srcs := src/overlays/movie/overlay_header.c

OVERLAYS += niki
overlay_niki_gcc_272_cdk_g0_srcs := src/overlays/niki/overlay_header.c src/overlays/niki/niki.c src/overlays/niki/niki_io.c

OVERLAYS += shop
overlay_shop_gcc_272_cdk_g0_srcs := \
	src/overlays/shop/overlay_header.c \
	src/overlays/shop/func_801429A4.c \
	src/overlays/shop/shop_init.c \
	src/overlays/shop/shop_update_interp.c \
	src/overlays/shop/shop_handle_list_input.c \
	src/overlays/shop/shop_element_utils.c \
	src/overlays/shop/func_80140E00.c \
	src/overlays/shop/shop_string_utils.c \
	src/overlays/shop/shop_element_init_a.c \
	src/overlays/shop/shop_element_init_b.c \
	src/overlays/shop/shop_mode_element_init.c \
	src/overlays/shop/shop_draw_indexed_glyph.c \
	src/overlays/shop/shop_draw_mode_glyph.c \
	src/overlays/shop/shop_draw_money_value.c \
	src/overlays/shop/shop_setup_default_list.c \
	src/overlays/shop/func_801415F4.c
overlay_shop_gcc_272_cdk_g0_nostrength_srcs := \
	src/overlays/shop/shop_setup_custom_list.c
overlay_shop_gcc_280_g0_srcs := \
	src/overlays/shop/unk1_mid_tail.c \
	src/overlays/shop/unk1_tail_mid.c

OVERLAYS += title
overlay_title_gcc_272_cdk_g0_srcs := \
	src/overlays/title/overlay_header.c \
	src/overlays/title/title.c \
	src/overlays/title/title_save.c

OVERLAYS += wsel
overlay_wsel_gcc_272_cdk_g0_srcs := \
	src/overlays/wsel/overlay_header.c \
	src/overlays/wsel/wsel_main_loop.c \
	src/overlays/wsel/wsel_init_display.c \
	src/overlays/wsel/func_800503F0.c \
	src/overlays/wsel/func_800520A8.c \
	src/overlays/wsel/func_80052154.c \
	src/overlays/wsel/func_800521D0.c \
	src/overlays/wsel/wsel_audio_fade.c \
	src/overlays/wsel/wsel_read_pad_state.c \
	src/overlays/wsel/func_80052384.c \
	src/overlays/wsel/wsel_read_pad_input.c \
	src/overlays/wsel/func_800514D8.c \
	src/overlays/wsel/func_800517BC.c
overlay_wsel_gcc_280_g0_srcs := \
	src/overlays/wsel/func_80050944.c \
	src/overlays/wsel/func_80050B40.c \
	src/overlays/wsel/func_80050DB0.c \
	src/overlays/wsel/func_80050F0C.c \
	src/overlays/wsel/func_800513D0.c \
	src/overlays/wsel/func_80051D78.c

OVERLAYS += zukan
# The overlay header is linked first. The category builder is compiled at -O0;
# the remaining ZUKAN code uses the CDK -O2 route.
overlay_zukan_gcc_272_cdk_g0_srcs := \
	src/overlays/zukan/overlay_header.c \
	src/overlays/zukan/zukan.c
overlay_zukan_gcc_280_g0_o0_srcs := \
	src/overlays/zukan/zukan_category.c
