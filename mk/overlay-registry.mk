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
	src/overlays/carda/func_801447DC.c \
	src/overlays/carda/func_80144A24.c \
	src/overlays/carda/carda_choice_prompt.c \
	src/overlays/carda/carda_choice_cancel.c \
	src/overlays/carda/carda_restore_record.c \
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
	src/overlays/carda/func_80142E10.c
overlay_carda_gcc_280_g0_srcs := \
	src/overlays/carda/carda_scan_hex_digits.c \
	src/overlays/carda/unk1_after_checksum_tail.c \
	src/overlays/carda/func_80145050.c \
	src/overlays/carda/unk1_after_choice_after_restore.c \
	src/overlays/carda/unk1_before_stream_reset_tail_after_fixed_prompts.c

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
overlay_field_gcc_272_gnu_g0_srcs := \
	src/overlays/field/func_800AEAC0.c \

overlay_field_gcc_272_cdk_g0_srcs := \
	src/overlays/field/func_800AF0E8.c \
	src/overlays/field/unk2_b_split018_tail.c \
	src/overlays/field/func_800A8128.c \
	src/overlays/field/unk2_b_split017_tail_tail.c \
	src/overlays/field/func_800A623C.c \
	src/overlays/field/func_80096E60.c \
	src/overlays/field/func_8009980C.c \
	src/overlays/field/unk2_i_b_split023_tail_tail.c \
	src/overlays/field/func_800AA570.c \
	src/overlays/field/func_800AD208.c \
	src/overlays/field/func_800A8660.c \
	src/overlays/field/func_8008AB2C.c \
	src/overlays/field/func_800952DC.c \
	src/overlays/field/unk2_i_b_split009_tail_after_951CC_tail.c \
	src/overlays/field/func_80091518.c \
	src/overlays/field/func_80090B38.c \
	src/overlays/field/func_80090D48.c \
	src/overlays/field/func_80087A9C.c \
	src/overlays/field/func_8008404C.c \
	src/overlays/field/func_800A6F1C.c \
	src/overlays/field/func_80098DD4.c \
	src/overlays/field/func_8008AFD8.c \
	src/overlays/field/func_800A9B88.c \
	src/overlays/field/func_8008C104.c \
	src/overlays/field/func_800878B4.c \
	src/overlays/field/func_80086FB8.c \
	src/overlays/field/func_8009615C.c \
	src/overlays/field/func_8008C2EC.c \
	src/overlays/field/field_text_quad.c \
	src/overlays/field/func_800A71CC.c \
	src/overlays/field/func_800A9E78.c \
	src/overlays/field/func_8009C12C.c \
	src/overlays/field/unk2_i_b_split013_tail.c \
	src/overlays/field/func_80091BC8.c \
	src/overlays/field/unk2_i_b_split008_after_91AC8_tail.c \
	src/overlays/field/func_800B941C.c \
	src/overlays/field/func_80091914.c \
	src/overlays/field/func_800A1F2C.c \
	src/overlays/field/unk2_i_b_split021_tail_tail.c \
	src/overlays/field/func_800AD524.c \
	src/overlays/field/unk2_b_split015_after_ad42c_tail.c \
	src/overlays/field/func_8008B73C.c \
	src/overlays/field/unk2_i_b_split005_tail.c \
	src/overlays/field/func_800ADCD0.c \
	src/overlays/field/func_80087FC0.c \
	src/overlays/field/func_8008C620.c \
	src/overlays/field/func_8008A580.c \
	src/overlays/field/unk2_i_b_split004_tail_b2_before_8a840_tail.c \
	src/overlays/field/func_800AA498.c \
	src/overlays/field/overlay_header.c \
	src/overlays/field/field_text_session_ops.c \
	src/overlays/field/field_actor_idle_ops.c \
	src/overlays/field/field_prim_builders.c \
	src/overlays/field/field_stream_buffer_ops.c \
	src/overlays/field/field_modal_fade_ops.c \
	src/overlays/field/field_actor_slot_lookup.c \
	src/overlays/field/field_actor_flag_ops.c \
	src/overlays/field/field_pad_context_ops.c \
	src/overlays/field/field_overlay_launchers.c \
	src/overlays/field/field4.c \
	src/overlays/field/field_audio.c \
	src/overlays/field/func_800675c8.c \
	src/overlays/field/func_80067aa4.c \
	src/overlays/field/field7.c \
	src/overlays/field/field8.c \
	src/overlays/field/field9.c \
	src/overlays/field/field10.c \
	src/overlays/field/field11.c \
	src/overlays/field/field12.c \
	src/overlays/field/field13.c \
	src/overlays/field/field14.c \
	src/overlays/field/field15.c \
	src/overlays/field/field16.c \
	src/overlays/field/field17.c \
	src/overlays/field/func_8009A204.c \
	src/overlays/field/func_8009CC60.c \
	src/overlays/field/func_8009CD30.c \
	src/overlays/field/func_8009CF84.c \
	src/overlays/field/func_8009D0D8.c \
	src/overlays/field/field18.c \
	src/overlays/field/field_find_free_actor_slot.c \
	src/overlays/field/field_actor_slot_queries.c \
	src/overlays/field/field_stop_actor_animations_for_object.c \
	src/overlays/field/field22.c \
	src/overlays/field/field27.c \
	src/overlays/field/field29.c \
	src/overlays/field/unk2.c \
	src/overlays/field/unk2_e.c \
	src/overlays/field/func_80083EEC.c \
	src/overlays/field/func_80084240.c \
	src/overlays/field/func_800842E0.c \
	src/overlays/field/func_80089980.c \
	src/overlays/field/func_8008B288.c \
	src/overlays/field/func_80084630.c \
	src/overlays/field/unk2_f.c \
	src/overlays/field/func_80085D30.c \
	src/overlays/field/unk2_h_b.c \
	src/overlays/field/field30.c \
	src/overlays/field/field32.c \
	src/overlays/field/field33.c \
	src/overlays/field/field35.c \
	src/overlays/field/field36.c \
	src/overlays/field/field37.c \
	src/overlays/field/field38.c \
	src/overlays/field/field39.c \
	src/overlays/field/field40.c \
	src/overlays/field/field41.c \
	src/overlays/field/field45.c \
	src/overlays/field/field46.c \
	src/overlays/field/field47.c \
	src/overlays/field/field48.c \
	src/overlays/field/field49.c \
	src/overlays/field/field50.c \
	src/overlays/field/field51.c \
	src/overlays/field/field52.c \
	src/overlays/field/field53.c \
	src/overlays/field/field56.c \
	src/overlays/field/field57.c \
	src/overlays/field/field307.c \
	src/overlays/field/field308.c \
	src/overlays/field/field320.c \
	src/overlays/field/field321.c \
	src/overlays/field/field329.c \
	src/overlays/field/field331.c \
	src/overlays/field/field332.c \
	src/overlays/field/field333.c \
	src/overlays/field/field337.c \
	src/overlays/field/field341.c \
	src/overlays/field/field342.c \
	src/overlays/field/field347.c \
	src/overlays/field/field349.c \
	src/overlays/field/field350.c \
	src/overlays/field/field352.c \
	src/overlays/field/func_800A4744.c \
	src/overlays/field/field64.c \
	src/overlays/field/field65.c \
	src/overlays/field/field66.c \
	src/overlays/field/field67.c \
	src/overlays/field/field68.c \
	src/overlays/field/field69.c \
	src/overlays/field/field70.c \
	src/overlays/field/field75.c \
	src/overlays/field/field76.c \
	src/overlays/field/field78.c \
	src/overlays/field/field79.c \
	src/overlays/field/field80.c \
	src/overlays/field/field_resource_load.c \
	src/overlays/field/field_layout_slot_state.c \
	src/overlays/field/func_80087CE0.c \
	src/overlays/field/func_80087E00.c \
	src/overlays/field/field_get_object_script_command.c \
	src/overlays/field/func_80088198.c \
	src/overlays/field/func_80089AE4.c \
	src/overlays/field/func_80089BE8.c \
	src/overlays/field/unk2_i_b_split004_tail_b_after_89be8.c \
	src/overlays/field/func_8008A4D0.c \
	src/overlays/field/func_8008A840.c \
	src/overlays/field/func_8008A9D8.c \
	src/overlays/field/func_8008AABC.c \
	src/overlays/field/func_8008AD44.c \
	src/overlays/field/field_queue_actor_animation_by_handle.c \
	src/overlays/field/func_8008C730.c \
	src/overlays/field/unk2_i_b_split006_tail2.c \
	src/overlays/field/func_8008D174.c \
	src/overlays/field/unk2_i_b_split006_tail.c \
	src/overlays/field/func_8008EF0C.c \
	src/overlays/field/func_8008D104.c \
	src/overlays/field/func_80090F80.c \
	src/overlays/field/unk2_i_b_split007_tail.c \
	src/overlays/field/unk2_i_b_split008.c \
	src/overlays/field/func_80091728.c \
	src/overlays/field/func_8009184C.c \
	src/overlays/field/func_80091AC8.c \
	src/overlays/field/func_800920FC.c \
	src/overlays/field/func_80092124.c \
	src/overlays/field/unk2_i_b_split008_tail2_tail.c \
	src/overlays/field/func_80092C98.c \
	src/overlays/field/unk2_i_b_split008_tail2b.c \
	src/overlays/field/func_80093EB4.c \
	src/overlays/field/unk2_i_b_split008_tail2b_after_93EB4.c \
	src/overlays/field/func_80094508.c \
	src/overlays/field/func_80092C24.c \
	src/overlays/field/func_80094690.c \
	src/overlays/field/unk2_i_b_split008_tail3.c \
	src/overlays/field/func_800949CC.c \
	src/overlays/field/func_80094B5C.c \
	src/overlays/field/unk2_i_b_split009.c \
	src/overlays/field/func_80094FDC.c \
	src/overlays/field/func_80095074.c \
	src/overlays/field/func_80095168.c \
	src/overlays/field/func_800951CC.c \
	src/overlays/field/func_8009A3E8.c \
	src/overlays/field/unk2_i_b_split011.c \
	src/overlays/field/func_8009BCF8.c \
	src/overlays/field/unk2_i_b_split013.c \
	src/overlays/field/func_8009C4B4.c \
	src/overlays/field/func_8009C620.c \
	src/overlays/field/func_8009C7B0.c \
	src/overlays/field/func_8009CA54.c \
	src/overlays/field/func_8009CB64.c \
	src/overlays/field/func_8009CE10.c \
	src/overlays/field/func_8009CF1C.c \
	src/overlays/field/unk2_i_b_split016_tail2.c \
	src/overlays/field/func_8009D9E0.c \
	src/overlays/field/func_8009E66C.c \
	src/overlays/field/func_8009FE54.c \
	src/overlays/field/unk2_i_b_split016_tail_b_tail_tail.c \
	src/overlays/field/unk2_i_b_split017.c \
	src/overlays/field/func_800A2990.c \
	src/overlays/field/unk2_i_b_split018.c \
	src/overlays/field/unk2_i_b_split019.c \
	src/overlays/field/field_restore_entry_music.c \
	src/overlays/field/func_800A39A8.c \
	src/overlays/field/func_800A3A90.c \
	src/overlays/field/unk2_b_split001_tail.c \
	src/overlays/field/func_800A3D44.c \
	src/overlays/field/func_800A3B78.c \
	src/overlays/field/unk2_b_split003.c \
	src/overlays/field/func_800A4798.c \
	src/overlays/field/unk2_b_split004.c \
	src/overlays/field/unk2_b_split004_b.c \
	src/overlays/field/func_800A5174.c \
	src/overlays/field/unk2_b_split004_b2.c \
	src/overlays/field/func_800A55E4.c \
	src/overlays/field/func_800A5670.c \
	src/overlays/field/unk2_b_split005.c \
	src/overlays/field/func_800A6060.c \
	src/overlays/field/unk2_b_split007.c \
	src/overlays/field/func_800A66B4.c \
	src/overlays/field/unk2_b_split007_b.c \
	src/overlays/field/func_800A7384.c \
	src/overlays/field/unk2_b_split009.c \
	src/overlays/field/func_800A764C.c \
	src/overlays/field/unk2_b_split009_b_before_a838c.c \
	src/overlays/field/func_800A788C.c \
	src/overlays/field/unk2_b_split009_b_before_a838c_after_788c.c \
	src/overlays/field/func_800A838C.c \
	src/overlays/field/unk2_b_split009_b_after_a838c.c \
	src/overlays/field/func_800A88A0.c \
	src/overlays/field/func_800A9198.c \
	src/overlays/field/unk2_b_split013_b_before_a9a5c.c \
	src/overlays/field/func_800A9A5C.c \
	src/overlays/field/func_800A9D70.c \
	src/overlays/field/func_800AA02C.c \
	src/overlays/field/unk2_b_split013_tail.c \
	src/overlays/field/unk2_b_split014.c \
	src/overlays/field/func_800AD42C.c \
	src/overlays/field/func_800AD120.c \
	src/overlays/field/func_800AD7DC.c \
	src/overlays/field/unk2_b_split015_tail.c \
	src/overlays/field/func_800ADF84.c \
	src/overlays/field/unk2_b_split016.c \
	src/overlays/field/func_800AE76C.c \
	src/overlays/field/func_800AE8A8.c \
	src/overlays/field/func_800AE9E0.c \
	src/overlays/field/func_800AF8E8.c \
	src/overlays/field/field_card_clock.c \
	src/overlays/field/func_800BF158.c \
	src/overlays/field/field_group_stat_transfer.c \
	src/overlays/field/field_select_distance_bucket.c \
	src/overlays/field/func_80087614.c \
	src/overlays/field/func_80087680.c \
	src/overlays/field/unk2_i_b_split020_tail.c \
	src/overlays/field/field242.c \
	src/overlays/field/func_800A1D98.c \
	src/overlays/field/func_800A20DC.c \
	src/overlays/field/unk2_b_split105_b.c \
	src/overlays/field/field_handle_actor_control_flag_40.c \
	src/overlays/field/unk2_b_split115.c \
	src/overlays/field/func_800AB774.c \
	src/overlays/field/field_coord_panels.c \
	src/overlays/field/func_8008AEB0.c \
	src/overlays/field/func_8008B1C8.c \
	src/overlays/field/func_8008AF68.c \
	src/overlays/field/func_8008B5D0.c \
	src/overlays/field/unk2_i_b_split023.c \
	src/overlays/field/func_80098FC4.c \
	src/overlays/field/unk2_i_b_split023_tail.c \
	src/overlays/field/func_8009A2A4.c \
	src/overlays/field/field_set_actor_horizontal_scale.c \
	src/overlays/field/field254.c \
	src/overlays/field/field255.c \
	src/overlays/field/field256.c \
	src/overlays/field/func_800C1EC8.c \
	src/overlays/field/field273.c \
	src/overlays/field/field_upload_initial_vram_resource.c \
	src/overlays/field/unk2_i_b_split024_tail.c \
	src/overlays/field/field288.c \
	src/overlays/field/field289.c \
	src/overlays/field/field290.c \
	src/overlays/field/field297.c \
	src/overlays/field/field298.c \
	src/overlays/field/func_800A8B90.c \
	src/overlays/field/func_8008BD88.c \
	src/overlays/field/func_8008BE38.c \
	src/overlays/field/func_8008C024.c \
	src/overlays/field/func_8008C4A8.c \
	src/overlays/field/func_80096A90.c \
	src/overlays/field/unk2_i_b_split023_mid_after_96A90.c \
	src/overlays/field/func_800970B0.c \
	src/overlays/field/unk2_i_b_split023_mid_tail.c \
	src/overlays/field/unk2_i_b_split023_mid_b_before_98c7c.c \
	src/overlays/field/func_80098C7C.c \

overlay_field_gcc_272_cdk_g0_nosched_srcs := src/overlays/field/field2.c
overlay_field_gcc_272_cdk_g0_noexpand_srcs := src/overlays/field/field_text_format_number.c
overlay_field_gcc_280_g0_srcs := \
	src/overlays/field/field_action_modifiers.c \
	src/overlays/field/field_record_stat_ops.c \
	src/overlays/field/field_script_flow_ops.c \
	src/overlays/field/field_script_primary_ops.c \
	src/overlays/field/field_script_pair_ops.c \
	src/overlays/field/field_script_extended_ops.c \
	src/overlays/field/field_script_operands.c \
	src/overlays/field/field_script_commands.c \
	src/overlays/field/field_record_growth_ops.c \
	src/overlays/field/field_record_effect_ops.c \
	src/overlays/field/field_active_record_ops.c \
	src/overlays/field/field_interaction_start.c \
	src/overlays/field/field_menu_action_runtime.c \
	src/overlays/field/field_menu_attribute_ops.c \
	src/overlays/field/func_800A2128.c \
	src/overlays/field/field_reward_command_ops.c \
	src/overlays/field/field_script_arith_ops.c \
	src/overlays/field/field_slot_pool_ops.c \
	src/overlays/field/field_state_ops.c \
	src/overlays/field/field_record_lookup_ops.c \
	src/overlays/field/field_stat_counter_ops.c \
	src/overlays/field/field_resource_table_ops.c \
	src/overlays/field/field_record_table_ops.c \
	src/overlays/field/field_menu_slot_ops.c \
	src/overlays/field/field_menu_object_ops.c \
	src/overlays/field/field_menu_record_ops.c \
	src/overlays/field/field_menu_record_setup.c \
	src/overlays/field/field_menu_record_audio_ops.c \
	src/overlays/field/field_actor_lifecycle.c \
	src/overlays/field/field_saved_slot_ops.c \
	src/overlays/field/field_actor_templates.c \
	src/overlays/field/field_action_descriptors.c \
	src/overlays/field/field_actor_record_ops.c \
	src/overlays/field/field_action_setup.c \
	src/overlays/field/field363.c \
	src/overlays/field/field364.c \
	src/overlays/field/field365.c \
	src/overlays/field/field368.c \
	src/overlays/field/field370.c \
	src/overlays/field/field_equipment_combination_rules.c \
	src/overlays/field/field_party_setup.c \
	src/overlays/field/func_800B4844.c \
	src/overlays/field/field_status_ticks.c \
	src/overlays/field/field_progression_ops.c \
	src/overlays/field/field_record_setup_ops.c \
	src/overlays/field/field_generated_record_ops.c \
	src/overlays/field/field_gosub_history_ops.c \
	src/overlays/field/field_golem_logic_blocks.c \
	src/overlays/field/field_group_layout_ops.c \
	src/overlays/field/field_menu_record_state.c \
	src/overlays/field/field_event_dispatch.c \
	src/overlays/field/field_menu_group_ops.c \
	src/overlays/field/field_menu_record_transfer.c
overlay_field_gcc_280_g4_srcs := src/overlays/field/field_fade.c
overlay_field_gcc_280_g4_noexpand_srcs := \
	src/overlays/field/field_scene_load.c \
	src/overlays/field/field_scene_build.c \
	src/overlays/field/field_render.c \
	src/overlays/field/field_animation.c \
	src/overlays/field/field_scene_api.c \
	src/overlays/field/field_collision.c \
	src/overlays/field/field_text.c

OVERLAYS += gname
overlay_gname_gcc_272_cdk_g0_srcs := src/overlays/gname/overlay_header.c src/overlays/gname/gname.c

OVERLAYS += golem
overlay_golem_gcc_272_cdk_g0_srcs := src/overlays/golem/overlay_header.c src/overlays/golem/golem.c

OVERLAYS += gosub
overlay_gosub_gcc_272_cdk_g0_srcs := src/overlays/gosub/overlay_header.c src/overlays/gosub/gosub.c

OVERLAYS += gover
overlay_gover_gcc_272_cdk_g0_srcs := src/overlays/gover/overlay_header.c src/overlays/gover/gover.c

OVERLAYS += menu
overlay_menu_gcc_272_cdk_g0_srcs := src/overlays/menu/overlay_header.c src/overlays/menu/menu.c

OVERLAYS += movie
overlay_movie_gcc_280_g4_srcs := src/overlays/movie/movie.c
# The header word is a plain -G0 const so it stays in .rodata.
overlay_movie_gcc_272_cdk_g0_srcs := src/overlays/movie/overlay_header.c

OVERLAYS += niki
overlay_niki_gcc_272_cdk_g0_srcs := src/overlays/niki/overlay_header.c src/overlays/niki/niki.c

OVERLAYS += shop
overlay_shop_gcc_272_cdk_g0_srcs := \
	src/overlays/shop/overlay_header.c \
	src/overlays/shop/func_801429A4.c \
	src/overlays/shop/shop_init.c \
	src/overlays/shop/shop_update_interp.c \
	src/overlays/shop/shop_element_utils.c \
	src/overlays/shop/func_80140E00.c \
	src/overlays/shop/shop_string_utils.c \
	src/overlays/shop/shop_element_init_a.c \
	src/overlays/shop/shop_element_init_b.c \
	src/overlays/shop/shop_mode_element_init.c \
	src/overlays/shop/shop_draw_indexed_glyph.c \
	src/overlays/shop/shop_draw_mode_glyph.c \
	src/overlays/shop/shop_draw_money_value.c \
	src/overlays/shop/shop_setup_default_list.c
overlay_shop_gcc_272_cdk_g0_nostrength_srcs := \
	src/overlays/shop/shop_setup_custom_list.c
overlay_shop_gcc_280_g0_srcs := \
	src/overlays/shop/unk1_after_interp.c \
	src/overlays/shop/unk1_mid_tail.c \
	src/overlays/shop/unk1_tail_mid.c

OVERLAYS += title
overlay_title_gcc_272_cdk_g0_srcs := src/overlays/title/overlay_header.c src/overlays/title/title.c

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
# The overlay header word stays in its own TU (linked first, before the asm
# rodata blob). zukan.c holds every optimized function - including the four
# formerly tagged gcc280_g0, which also match under gcc272_cdk. func_80142D08 is
# the only -O0 function, so it keeps its own TU.
overlay_zukan_gcc_272_cdk_g0_srcs := \
	src/overlays/zukan/overlay_header.c \
	src/overlays/zukan/zukan.c
overlay_zukan_gcc_280_g0_o0_srcs := \
	src/overlays/zukan/func_80142D08.c
