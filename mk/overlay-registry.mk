# ============================================================================
# Overlay registry and per-source toolchain routing
# ============================================================================

# Register an overlay by adding its lowercase directory name to OVERLAYS.
# Every direct .c file under src/overlays/<name>/ must appear in exactly one
# toolchain configuration:
#
#   overlay_<name>_gcc_272_cdk_g0_srcs
#   overlay_<name>_gcc_272_gnu_g0_srcs
#   overlay_<name>_gcc_280_g0_srcs
#   overlay_<name>_gcc_280_g0_o0_srcs
#   overlay_<name>_gcc_280_g4_srcs
#   overlay_<name>_gcc_280_g4_noexpand_srcs
#
# overlays.mk rejects missing, unknown, or multiply routed sources. If a linker
# script expects a standalone assets/<name>.o, define (path under the version's
# assets tree):
#
#   overlay_<name>_asset_src := $(ASSETS_DIR)/<name>.bin
#
# Splat databin assets referenced through .incbin do not use this setting.

OVERLAYS += addhero
overlay_addhero_gcc_272_cdk_g0_srcs := \
	src/overlays/addhero/overlay_header.c \
	src/overlays/addhero/addhero.c \
	src/overlays/addhero/addhero_widgets.c \
	src/overlays/addhero/addhero_card.c \
	src/overlays/addhero/addhero_glyph.c

OVERLAYS += carda
overlay_carda_gcc_272_cdk_g0_srcs := \
	src/overlays/carda/overlay_header.c \
	src/overlays/carda/carda.c \
	src/overlays/carda/carda_widgets.c \
	src/overlays/carda/carda_save.c \
	src/overlays/carda/carda_card.c \
	src/overlays/carda/carda_glyph.c

OVERLAYS += checkps
overlay_checkps_gcc_272_cdk_g0_srcs := \
	src/overlays/checkps/overlay_header.c \
	src/overlays/checkps/init.c \
	src/overlays/checkps/font.c
overlay_checkps_gcc_280_g0_srcs := src/overlays/checkps/kanji.c
overlay_checkps_gcc_272_gnu_g0_srcs := \
	src/overlays/checkps/pattern.c \
	src/overlays/checkps/cdrom.c
# Preserve GCC/local switch labels for objdiff.  cdrom.c's only compiler-emitted
# .data is the control-flow anchor array, so discard it and recreate the target
# object's empty .data section without contributing any linked bytes.  -G0 keeps
# cdrom.c's static CD-state variables in .bss (GNU as would otherwise use .sbss).
overlay_checkps_gcc_272_gnu_as_extra_flags_cdrom := -L -G0
overlay_checkps_gcc_272_gnu_objcopy_flags_cdrom := --remove-section=.data --remove-section=.text --rename-section=.text.cdrom=.text --add-section=.data=/dev/null --set-section-flags=.data=alloc,data
overlay_checkps_target_as_extra_flags_cdrom := -L

OVERLAYS += cload
overlay_cload_gcc_272_cdk_g0_srcs := \
	src/overlays/cload/overlay_header.c \
	src/overlays/cload/cload.c \
	src/overlays/cload/cload_widgets.c \
	src/overlays/cload/cload_card.c \
	src/overlays/cload/cload_glyph.c

OVERLAYS += field
# Sources follow FIELD.BIN.yaml address order within each compiler configuration.
overlay_field_gcc_272_cdk_g0_srcs := \
	src/overlays/field/overlay_header.c \
	src/overlays/field/field_frame_commands.c \
	src/overlays/field/field_subsystem_init.c \
	src/overlays/field/field_draw_state.c \
	src/overlays/field/field_fade.c \
	src/overlays/field/field_actor_runtime.c \
	src/overlays/field/field_effect_update.c \
	src/overlays/field/field_mesh_render.c \
	src/overlays/field/field_mesh_transform.c \
	src/overlays/field/field_mesh_part_animation.c \
	src/overlays/field/field_actor_slot_resources.c \
	src/overlays/field/field_actor_hud_effects.c \
	src/overlays/field/field_actor_key_ops.c \
	src/overlays/field/field_actor_script_ops.c \
	src/overlays/field/field_actor_reactions.c \
	src/overlays/field/field_actor_behavior.c \
	src/overlays/field/field_actor_input_map_init.c \
	src/overlays/field/field_actor_action_defaults.c \
	src/overlays/field/field_actor_input_actions.c \
	src/overlays/field/field_actor_camera.c \
	src/overlays/field/field_actor_idle_ops.c \
	src/overlays/field/field_actor_motion.c \
	src/overlays/field/field_actor_state_updates.c \
	src/overlays/field/field_actor_transition_reset.c \
	src/overlays/field/field_contact_geometry.c \
	src/overlays/field/field_actor_resource_unpack.c \
	src/overlays/field/field_scene_transition.c \
	src/overlays/field/field_text_window_api.c \
	src/overlays/field/field_block_allocator.c \
	src/overlays/field/field_actor_target_queries.c \
	src/overlays/field/field_actor_effects.c \
	src/overlays/field/field_path_interpolation.c \
	src/overlays/field/field_command_history.c \
	src/overlays/field/field_pair_indicators.c \
	src/overlays/field/field_audio_runtime.c \
	src/overlays/field/field_ring_selection.c \
	src/overlays/field/field_dialog_screens.c \
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

overlay_field_gcc_280_g0_srcs := \
	src/overlays/field/field_interaction_start.c \
	src/overlays/field/field_event_dispatch.c \
	src/overlays/field/field_party_state.c \
	src/overlays/field/field_actor_templates.c \
	src/overlays/field/field_actor_lifecycle.c \
	src/overlays/field/field_status_ticks.c \
	src/overlays/field/field_action_descriptors.c \
	src/overlays/field/field_action_setup.c \
	src/overlays/field/field_progression_ops.c \
	src/overlays/field/field_action_modifiers.c \
	src/overlays/field/field_record_stat_ops.c \
	src/overlays/field/field_script_ops.c \
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
	src/overlays/field/field_menu_ops.c \
	src/overlays/field/field_select_distance_bucket.c \
	src/overlays/field/field_layout_slot_state.c \
	src/overlays/field/field_equipment_combination_rules.c \
	src/overlays/field/field_golem_logic_blocks.c

overlay_field_gcc_280_g4_noexpand_srcs := \
	src/overlays/field/field_scene_load.c \
	src/overlays/field/field_scene_build.c \
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
	src/overlays/shop/shop.c \
	src/overlays/shop/shop_render.c \
	src/overlays/shop/shop_text.c \
	src/overlays/shop/shop_trade.c

OVERLAYS += title
overlay_title_gcc_272_cdk_g0_srcs := \
	src/overlays/title/overlay_header.c \
	src/overlays/title/title.c \
	src/overlays/title/title_save.c

OVERLAYS += wmap
overlay_wmap_gcc_280_g0_srcs := \
	src/overlays/wmap/wmap_map_display.c \
	src/overlays/wmap/wmap_party_travel.c \
	src/overlays/wmap/wmap_land_selection.c \
	src/overlays/wmap/wmap_land_effect_loader.c \
	src/overlays/wmap/wmap_land_transition.c \
	src/overlays/wmap/wmap_land_preview.c \
	src/overlays/wmap/wmap_land_preview_lines.c \
	src/overlays/wmap/wmap_land_layout.c \
	src/overlays/wmap/wmap_map_labels.c \
	src/overlays/wmap/wmap_main.c \
	src/overlays/wmap/wmap_frame_render.c \
	src/overlays/wmap/wmap_resource_support.c \
	src/overlays/wmap/wmap_view_effects.c \
	src/overlays/wmap/wmap_sprite_render.c \
	src/overlays/wmap/wmap_model_render.c \
	src/overlays/wmap/wmap_effect_primitives.c \
	src/overlays/wmap/wmap_sequence_runtime.c \
	src/overlays/wmap/wmap_effect_backdrop.c \
	src/overlays/wmap/wmap_land_effect_18.c \
	src/overlays/wmap/wmap_land_effect_04.c \
	src/overlays/wmap/wmap_land_effect_01.c \
	src/overlays/wmap/wmap_land_effect_13.c \
	src/overlays/wmap/wmap_land_effect_26.c \
	src/overlays/wmap/wmap_land_effect_32.c \
	src/overlays/wmap/wmap_land_effect_15.c \
	src/overlays/wmap/wmap_land_effect_02.c \
	src/overlays/wmap/wmap_land_effect_07.c \
	src/overlays/wmap/wmap_land_effect_30.c \
	src/overlays/wmap/wmap_land_effect_12.c \
	src/overlays/wmap/wmap_land_effect_11.c \
	src/overlays/wmap/wmap_land_effect_03.c \
	src/overlays/wmap/wmap_land_effect_21.c \
	src/overlays/wmap/wmap_land_effect_17.c \
	src/overlays/wmap/wmap_land_effect_05.c \
	src/overlays/wmap/wmap_land_effect_10.c \
	src/overlays/wmap/wmap_land_effect_00.c \
	src/overlays/wmap/wmap_land_effect_08.c \
	src/overlays/wmap/wmap_land_effect_09.c \
	src/overlays/wmap/wmap_land_effect_16.c \
	src/overlays/wmap/wmap_travel_sequences.c \
	src/overlays/wmap/wmap_land_effect_27.c \
	src/overlays/wmap/wmap_land_effect_23.c \
	src/overlays/wmap/wmap_land_effect_22.c \
	src/overlays/wmap/wmap_land_effect_19.c \
	src/overlays/wmap/wmap_map_events.c \
	src/overlays/wmap/wmap_land_event_17.c \
	src/overlays/wmap/wmap_land_effect_25.c \
	src/overlays/wmap/wmap_land_event_00_a.c \
	src/overlays/wmap/wmap_land_event_00_b.c \
	src/overlays/wmap/wmap_land_effect_31.c \
	src/overlays/wmap/wmap_land_effect_24.c \
	src/overlays/wmap/wmap_land_effect_33.c \
	src/overlays/wmap/wmap_special_effect_34.c \
	src/overlays/wmap/wmap_special_effect_35.c

overlay_wmap_gcc_280_g0_o0_srcs := \
	src/overlays/wmap/wmap_effect_resources.c \
	src/overlays/wmap/wmap_pathfinding.c

OVERLAYS += wsel
overlay_wsel_gcc_272_cdk_g0_srcs := \
	src/overlays/wsel/overlay_header.c \
	src/overlays/wsel/wsel.c

OVERLAYS += zukan
# The overlay header is linked first. The category builder is compiled at -O0;
# the remaining ZUKAN code uses the CDK -O2 route.
overlay_zukan_gcc_272_cdk_g0_srcs := \
	src/overlays/zukan/overlay_header.c \
	src/overlays/zukan/zukan.c
overlay_zukan_gcc_280_g0_o0_srcs := \
	src/overlays/zukan/zukan_category.c
