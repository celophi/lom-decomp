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
overlay_addhero_gcc_272_cdk_g0_srcs := src/overlays/addhero/overlay_header.c src/overlays/addhero/addhero.c

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
overlay_field_gcc_272_cdk_g0_srcs := \
	src/overlays/field/func_8008404C.c \
	src/overlays/field/func_800A6F1C.c \
	src/overlays/field/func_80098DD4.c \
	src/overlays/field/func_8008AFD8.c \
	src/overlays/field/func_800A9B88.c \
	src/overlays/field/func_8008C104.c \
	src/overlays/field/func_800878B4.c \
	src/overlays/field/unk2_i_b_split020_tail_tail.c \
	src/overlays/field/func_80086FB8.c \
	src/overlays/field/unk2_b_split068_tail_tail.c \
	src/overlays/field/func_800B7D10.c \
	src/overlays/field/unk2_b_split029_tail_tail.c \
	src/overlays/field/func_8009615C.c \
	src/overlays/field/unk2_b_split081_tail.c \
	src/overlays/field/func_800BFF90.c \
	src/overlays/field/func_8008C2EC.c \
	src/overlays/field/func_800AFC50.c \
	src/overlays/field/unk2_b_split019_tail.c \
	src/overlays/field/func_800CB758.c \
	src/overlays/field/func_800A71CC.c \
	src/overlays/field/func_800A9E78.c \
	src/overlays/field/func_800BCCE0.c \
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
	src/overlays/field/func_800B3DF4.c \
	src/overlays/field/unk2_b_split025_tail2b_tail.c \
	src/overlays/field/func_800C80BC.c \
	src/overlays/field/func_800ADCD0.c \
	src/overlays/field/func_80087FC0.c \
	src/overlays/field/unk2_b_split053_b_tail.c \
	src/overlays/field/func_8008C620.c \
	src/overlays/field/func_8008A580.c \
	src/overlays/field/unk2_i_b_split004_tail_b2_before_8a840_tail.c \
	src/overlays/field/func_800CBD70.c \
	src/overlays/field/func_800AA498.c \
	src/overlays/field/unk2_b_split013_tail_tail.c \
	src/overlays/field/func_800BF730.c \
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
	src/overlays/field/func_800CA1E0.c \
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
	src/overlays/field/field310.c \
	src/overlays/field/field_reset_menu_action_slot.c \
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
	src/overlays/field/field81.c \
	src/overlays/field/field83.c \
	src/overlays/field/field84.c \
	src/overlays/field/field86.c \
	src/overlays/field/field90.c \
	src/overlays/field/field93.c \
	src/overlays/field/field94.c \
	src/overlays/field/field123.c \
	src/overlays/field/field147.c \
	src/overlays/field/field168.c \
	src/overlays/field/field178.c \
	src/overlays/field/field187.c \
	src/overlays/field/field_load_selected_small_history_value.c \
	src/overlays/field/field_reset_music_track_index.c \
	src/overlays/field/field194.c \
	src/overlays/field/field198.c \
	src/overlays/field/field203.c \
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
	src/overlays/field/unk2_i_b_split004_tail2.c \
	src/overlays/field/func_8008AABC.c \
	src/overlays/field/func_8008AD44.c \
	src/overlays/field/field_queue_actor_animation_by_handle.c \
	src/overlays/field/func_8008C730.c \
	src/overlays/field/unk2_i_b_split006_tail2.c \
	src/overlays/field/func_8008D174.c \
	src/overlays/field/unk2_i_b_split006_tail.c \
	src/overlays/field/func_8008EF0C.c \
	src/overlays/field/unk2_i_b_split006_tail_after_8EF0C.c \
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
	src/overlays/field/unk2_i_b_split009_tail_after_951CC.c \
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
	src/overlays/field/unk2_b_split006.c \
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
	src/overlays/field/unk2_b_split015_before_ad42c.c \
	src/overlays/field/func_800AD42C.c \
	src/overlays/field/func_800AD120.c \
	src/overlays/field/func_800AD7DC.c \
	src/overlays/field/unk2_b_split015_tail.c \
	src/overlays/field/func_800ADF84.c \
	src/overlays/field/unk2_b_split016.c \
	src/overlays/field/func_800AE76C.c \
	src/overlays/field/func_800AE8A8.c \
	src/overlays/field/func_800AE9E0.c \
	src/overlays/field/unk2_b_split017_tail.c \
	src/overlays/field/unk2_b_split018.c \
	src/overlays/field/func_800AF8E8.c \
	src/overlays/field/unk2_b_split019.c \
	src/overlays/field/func_800B0094.c \
	src/overlays/field/unk2_b_split020.c \
	src/overlays/field/func_800B0710.c \
	src/overlays/field/func_800B0888.c \
	src/overlays/field/unk2_b_split021_tail_before_b0a08.c \
	src/overlays/field/func_800B0A08.c \
	src/overlays/field/unk2_b_split022_after_b0c54.c \
	src/overlays/field/unk2_b_split022_b_after_b0efc.c \
	src/overlays/field/unk2_b_split022_b_tail_after_1BBC_before_1f10.c \
	src/overlays/field/unk2_b_split022_b_tail_after_1f10_after_b2198.c \
	src/overlays/field/unk2_b_split025_tail2_c.c \
	src/overlays/field/func_800B4E60.c \
	src/overlays/field/unk2_b_split026_after_4f80.c \
	src/overlays/field/unk2_b_split028_after_729c.c \
	src/overlays/field/unk2_b_split029_mid.c \
	src/overlays/field/func_800B7A74.c \
	src/overlays/field/unk2_b_split031_mid.c \
	src/overlays/field/unk2_b_split034.c \
	src/overlays/field/unk2_b_split048.c \
	src/overlays/field/unk2_b_split051.c \
	src/overlays/field/func_800BF158.c \
	src/overlays/field/unk2_b_split053.c \
	src/overlays/field/unk2_b_split053_after_bfe70_after_c015c.c \
	src/overlays/field/unk2_b_split053_b.c \
	src/overlays/field/func_800C0C74.c \
	src/overlays/field/unk2_b_split055.c \
	src/overlays/field/func_800C10F0.c \
	src/overlays/field/unk2_b_split056.c \
	src/overlays/field/unk2_b_split056_b.c \
	src/overlays/field/unk2_b_split064.c \
	src/overlays/field/unk2_b_split064_tail.c \
	src/overlays/field/func_800C3CB4.c \
	src/overlays/field/func_800C4364.c \
	src/overlays/field/unk2_b_split069.c \
	src/overlays/field/unk2_b_split071.c \
	src/overlays/field/unk2_b_split073_tail.c \
	src/overlays/field/unk2_b_split075.c \
	src/overlays/field/func_800C6A30.c \
	src/overlays/field/func_800C7F44.c \
	src/overlays/field/unk2_b_split082_after_8260.c \
	src/overlays/field/unk2_b_split083.c \
	src/overlays/field/unk2_b_split087_tail.c \
	src/overlays/field/unk2_b_split088_after_c9dcc.c \
	src/overlays/field/func_800CB918.c \
	src/overlays/field/unk2_b_split098_after_cb918.c \
	src/overlays/field/func_80087614.c \
	src/overlays/field/func_80087680.c \
	src/overlays/field/unk2_i_b_split020_tail.c \
	src/overlays/field/field242.c \
	src/overlays/field/func_800A1D98.c \
	src/overlays/field/func_800A20DC.c \
	src/overlays/field/unk2_b_split105_b.c \
	src/overlays/field/unk2_b_split107_tail_b_after_9ea8.c \
	src/overlays/field/unk2_b_split112.c \
	src/overlays/field/field_handle_actor_control_flag_40.c \
	src/overlays/field/unk2_b_split115.c \
	src/overlays/field/func_800AB774.c \
	src/overlays/field/field_coord_panels.c \
	src/overlays/field/unk2_b_split118.c \
	src/overlays/field/unk2_b_split119.c \
	src/overlays/field/unk2_b_split120.c \
	src/overlays/field/unk2_b_split120_after_bdfd4.c \
	src/overlays/field/unk2_b_split124.c \
	src/overlays/field/unk2_b_split125.c \
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
	src/overlays/field/field268.c \
	src/overlays/field/field273.c \
	src/overlays/field/field_upload_initial_vram_resource.c \
	src/overlays/field/unk2_i_b_split024_tail.c \
	src/overlays/field/field288.c \
	src/overlays/field/field289.c \
	src/overlays/field/field290.c \
	src/overlays/field/field297.c \
	src/overlays/field/field298.c \
	src/overlays/field/func_800A8B90.c \
	src/overlays/field/func_800B7B98.c \
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
	src/overlays/field/unk2_b_split026_after_543c.c

overlay_field_gcc_272_cdk_g0_nosched_srcs := src/overlays/field/field2.c
overlay_field_gcc_272_cdk_g0_noexpand_srcs := src/overlays/field/field_text_format_number.c
overlay_field_gcc_280_g0_srcs := \
	src/overlays/field/func_800B2654.c \
	src/overlays/field/func_800BD778.c \
	src/overlays/field/func_800C3D38.c \
	src/overlays/field/func_800B2B54.c \
	src/overlays/field/func_800C5BCC.c \
	src/overlays/field/func_800C766C.c \
	src/overlays/field/func_800C96C4.c \
	src/overlays/field/func_800B5A88.c \
	src/overlays/field/func_800B4684.c \
	src/overlays/field/func_800B28E0.c \
	src/overlays/field/func_800B8308.c \
	src/overlays/field/equipment_pair_has_classes.c \
	src/overlays/field/equipment_combination_find.c \
	src/overlays/field/func_800A2128.c \
	src/overlays/field/func_800BF514.c \
	src/overlays/field/func_800B4410.c \
	src/overlays/field/func_800C2724.c \
	src/overlays/field/func_800B49C0.c \
	src/overlays/field/func_800C7340.c \
	src/overlays/field/func_800B99A8.c \
	src/overlays/field/func_800C0A38.c \
	src/overlays/field/func_800C1A18.c \
	src/overlays/field/func_800BDF00.c \
	src/overlays/field/field_script_arith_ops.c \
	src/overlays/field/field_slot_pool_ops.c \
	src/overlays/field/field_script_actor_ops.c \
	src/overlays/field/field_script_owner_ops.c \
	src/overlays/field/field_state_block_ops.c \
	src/overlays/field/field_state_ops.c \
	src/overlays/field/field_script_misc_ops.c \
	src/overlays/field/field_record_lookup_ops.c \
	src/overlays/field/field_stat_counter_ops.c \
	src/overlays/field/field_resource_table_ops.c \
	src/overlays/field/field_record_table_ops.c \
	src/overlays/field/field_menu_slot_ops.c \
	src/overlays/field/field_menu_count_ops.c \
	src/overlays/field/field_gosub_result_ops.c \
	src/overlays/field/field_menu_action_slots.c \
	src/overlays/field/field_menu_record_ops.c \
	src/overlays/field/unk2_b_split082_b.c \
	src/overlays/field/func_800B622C.c \
	src/overlays/field/func_800BD318.c \
	src/overlays/field/func_800BDBAC.c \
	src/overlays/field/func_800BE404.c \
	src/overlays/field/func_800C8FA8.c \
	src/overlays/field/field303.c \
	src/overlays/field/field309.c \
	src/overlays/field/field325.c \
	src/overlays/field/field343.c \
	src/overlays/field/field345.c \
	src/overlays/field/func_800C2264.c \
	src/overlays/field/func_800C23F4.c \
	src/overlays/field/func_800B0AF8.c \
	src/overlays/field/func_800B0C54.c \
	src/overlays/field/func_800B168C.c \
	src/overlays/field/func_800B177C.c \
	src/overlays/field/func_800B1894.c \
	src/overlays/field/func_800BDFD4.c \
	src/overlays/field/func_800B0EFC.c \
	src/overlays/field/func_800C9DCC.c \
	src/overlays/field/func_800B1AA8.c \
	src/overlays/field/func_800B1BBC.c \
	src/overlays/field/func_800B1F10.c \
	src/overlays/field/func_800B20B4.c \
	src/overlays/field/func_800B2198.c \
	src/overlays/field/func_800B42B4.c \
	src/overlays/field/func_800B4584.c \
	src/overlays/field/func_800B543C.c \
	src/overlays/field/func_800B6C48.c \
	src/overlays/field/func_800B6D3C.c \
	src/overlays/field/func_800B9278.c \
	src/overlays/field/func_800B95EC.c \
	src/overlays/field/func_800B977C.c \
	src/overlays/field/func_800B9868.c \
	src/overlays/field/func_800B9AC4.c \
	src/overlays/field/func_800C24BC.c \
	src/overlays/field/func_800C2640.c \
	src/overlays/field/func_800B5948.c \
	src/overlays/field/func_800B5C54.c \
	src/overlays/field/func_800B5D60.c \
	src/overlays/field/func_800B5E5C.c \
	src/overlays/field/func_800B5F60.c \
	src/overlays/field/field_script_read_operand.c \
	src/overlays/field/func_800B7C58.c \
	src/overlays/field/func_800B820C.c \
	src/overlays/field/func_800B84B4.c \
	src/overlays/field/field371.c \
	src/overlays/field/field373.c \
	src/overlays/field/field356.c \
	src/overlays/field/func_800BF3D8.c \
	src/overlays/field/field358.c \
	src/overlays/field/field362.c \
	src/overlays/field/field363.c \
	src/overlays/field/field364.c \
	src/overlays/field/field365.c \
	src/overlays/field/field368.c \
	src/overlays/field/field370.c \
	src/overlays/field/equipment_combination_quantity.c \
	src/overlays/field/field_equipment_combination_rules.c \
	src/overlays/field/func_800B3D84.c \
	src/overlays/field/func_800B4844.c \
	src/overlays/field/func_800B4B44.c \
	src/overlays/field/func_800B4CE4.c \
	src/overlays/field/func_800B4D1C.c \
	src/overlays/field/func_800B4DF0.c \
	src/overlays/field/func_800B4F80.c \
	src/overlays/field/func_800B607C.c \
	src/overlays/field/func_800B60DC.c \
	src/overlays/field/func_800B70F4.c \
	src/overlays/field/func_800B7164.c \
	src/overlays/field/func_800B729C.c \
	src/overlays/field/func_800B76F8.c \
	src/overlays/field/func_800B69B0.c \
	src/overlays/field/func_800B6EC0.c \
	src/overlays/field/func_800BB3D8.c \
	src/overlays/field/func_800BE710.c \
	src/overlays/field/func_800BE888.c \
	src/overlays/field/func_800C7DB8.c \
	src/overlays/field/func_800B7020.c \
	src/overlays/field/func_800C8964.c \
	src/overlays/field/func_800C94F4.c \
	src/overlays/field/func_800B78C0.c \
	src/overlays/field/func_800BD434.c \
	src/overlays/field/func_800BD4A8.c \
	src/overlays/field/func_800BD55C.c \
	src/overlays/field/func_800BD650.c \
	src/overlays/field/func_800BF68C.c \
	src/overlays/field/func_800BFE70.c \
	src/overlays/field/func_800C015C.c \
	src/overlays/field/func_800C14A4.c \
	src/overlays/field/func_800C2848.c \
	src/overlays/field/func_800C2D08.c \
	src/overlays/field/func_800C3A00.c \
	src/overlays/field/func_800C3BD8.c \
	src/overlays/field/func_800C5760.c \
	src/overlays/field/func_800C7494.c \
	src/overlays/field/func_800C7090.c \
	src/overlays/field/func_800BD3B0.c \
	src/overlays/field/field_golem_logic_blocks.c \
	src/overlays/field/field_script_ops_00_01.c \
	src/overlays/field/func_800B8684.c \
	src/overlays/field/field_script_ops_03_08.c \
	src/overlays/field/func_800B8B80.c \
	src/overlays/field/field_script_op_0a.c \
	src/overlays/field/field_script_op_0e.c \
	src/overlays/field/field_script_ops_17_1c.c \
	src/overlays/field/field_script_ops_1e_36.c \
	src/overlays/field/field_script_ops_38_3f.c \
	src/overlays/field/func_800CBC0C.c \
	src/overlays/field/func_800CBE64.c \
	src/overlays/field/func_800CBEC4.c \
	src/overlays/field/field91.c \
	src/overlays/field/field99.c \
	src/overlays/field/field124.c \
	src/overlays/field/field125.c \
	src/overlays/field/func_800BD6F4.c \
	src/overlays/field/field126.c \
	src/overlays/field/field127.c \
	src/overlays/field/field129.c \
	src/overlays/field/field130.c \
	src/overlays/field/field143.c \
	src/overlays/field/field145.c \
	src/overlays/field/field148.c \
	src/overlays/field/func_800C15AC.c \
	src/overlays/field/field149.c \
	src/overlays/field/field167.c \
	src/overlays/field/field169.c \
	src/overlays/field/field170.c \
	src/overlays/field/func_800C5B64.c \
	src/overlays/field/field173.c \
	src/overlays/field/field179.c \
	src/overlays/field/field180.c \
	src/overlays/field/field188.c \
	src/overlays/field/func_800C7558.c \
	src/overlays/field/func_800C8014.c \
	src/overlays/field/func_800C8260.c \
	src/overlays/field/field190.c \
	src/overlays/field/field191.c \
	src/overlays/field/field193.c \
	src/overlays/field/field197.c \
	src/overlays/field/field199.c \
	src/overlays/field/field236.c \
	src/overlays/field/func_800B19FC.c \
	src/overlays/field/func_800B286C.c \
	src/overlays/field/func_800B2A9C.c \
	src/overlays/field/func_800B6B28.c \
	src/overlays/field/func_800B8CFC.c \
	src/overlays/field/field260.c \
	src/overlays/field/field263.c \
	src/overlays/field/field267.c \
	src/overlays/field/field270.c \
	src/overlays/field/field271.c \
	src/overlays/field/field274.c \
	src/overlays/field/field293.c \
	src/overlays/field/field296.c \
	src/overlays/field/field299.c \
	src/overlays/field/field301.c \
	src/overlays/field/unk2_b_split126.c
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
