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
overlay_addhero_gcc_272_cdk_g0_srcs := src/overlays/addhero/addhero.c

OVERLAYS += carda
overlay_carda_gcc_272_cdk_g0_srcs := \
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
	src/overlays/carda/func_80149A4C.c
overlay_carda_gcc_280_g0_srcs := \
	src/overlays/carda/unk1_after_strings.c \
	src/overlays/carda/carda_scan_hex_digits.c \
	src/overlays/carda/unk1_after_checksum_tail.c \
	src/overlays/carda/unk1_after_choice.c \
	src/overlays/carda/unk1_after_choice_after_restore.c \
	src/overlays/carda/unk1_before_stream_reset_tail_after_fixed_prompts.c

OVERLAYS += checkps
overlay_checkps_gcc_272_cdk_g0_srcs := \
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
	src/overlays/cload/cload.c 

OVERLAYS += field
overlay_field_gcc_272_cdk_g0_srcs := \
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
	src/overlays/field/field24.c \
	src/overlays/field/field25.c \
	src/overlays/field/field26.c \
	src/overlays/field/field27.c \
	src/overlays/field/field29.c \
	src/overlays/field/unk2.c \
	src/overlays/field/unk2_e.c \
	src/overlays/field/func_80084240.c \
	src/overlays/field/unk2_e_tail.c \
	src/overlays/field/func_80084630.c \
	src/overlays/field/unk2_f.c \
	src/overlays/field/unk2_f_b.c \
	src/overlays/field/func_80086030.c \
	src/overlays/field/func_800860CC.c \
	src/overlays/field/field_load_vram_resource.c \
	src/overlays/field/unk2_h_b.c \
	src/overlays/field/unk2_i.c \
	src/overlays/field/func_80086F48.c \
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
	src/overlays/field/field62.c \
	src/overlays/field/field63.c \
	src/overlays/field/field302.c \
	src/overlays/field/field307.c \
	src/overlays/field/field308.c \
	src/overlays/field/field310.c \
	src/overlays/field/field_reset_menu_action_slot.c \
	src/overlays/field/field320.c \
	src/overlays/field/field321.c \
	src/overlays/field/field322.c \
	src/overlays/field/field323.c \
	src/overlays/field/field326.c \
	src/overlays/field/field327.c \
	src/overlays/field/field328.c \
	src/overlays/field/field329.c \
	src/overlays/field/field330.c \
	src/overlays/field/field331.c \
	src/overlays/field/field332.c \
	src/overlays/field/field333.c \
	src/overlays/field/field337.c \
	src/overlays/field/field340.c \
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
	src/overlays/field/field71.c \
	src/overlays/field/field72.c \
	src/overlays/field/field73.c \
	src/overlays/field/field74.c \
	src/overlays/field/field75.c \
	src/overlays/field/field76.c \
	src/overlays/field/field78.c \
	src/overlays/field/field79.c \
	src/overlays/field/field80.c \
	src/overlays/field/field81.c \
	src/overlays/field/field83.c \
	src/overlays/field/field84.c \
	src/overlays/field/field86.c \
	src/overlays/field/field88.c \
	src/overlays/field/field90.c \
	src/overlays/field/field93.c \
	src/overlays/field/field94.c \
	src/overlays/field/field108.c \
	src/overlays/field/field111.c \
	src/overlays/field/field118.c \
	src/overlays/field/field120.c \
	src/overlays/field/field123.c \
	src/overlays/field/field132.c \
	src/overlays/field/field140.c \
	src/overlays/field/field144.c \
	src/overlays/field/field147.c \
	src/overlays/field/field153.c \
	src/overlays/field/field158.c \
	src/overlays/field/field160.c \
	src/overlays/field/field162.c \
	src/overlays/field/field164.c \
	src/overlays/field/field165.c \
	src/overlays/field/field168.c \
	src/overlays/field/field178.c \
	src/overlays/field/field185.c \
	src/overlays/field/field187.c \
	src/overlays/field/field_load_selected_small_history_value.c \
	src/overlays/field/field_reset_music_track_index.c \
	src/overlays/field/field194.c \
	src/overlays/field/field198.c \
	src/overlays/field/field203.c \
	src/overlays/field/func_80087CE0.c \
	src/overlays/field/func_80087E00.c \
	src/overlays/field/unk2_i_b_split003.c \
	src/overlays/field/field_get_object_script_command.c \
	src/overlays/field/func_80088198.c \
	src/overlays/field/unk2_i_b_split004_tail.c \
	src/overlays/field/unk2_i_b_split004_tail_b.c \
	src/overlays/field/func_8008A4D0.c \
	src/overlays/field/unk2_i_b_split004_tail_b2.c \
	src/overlays/field/unk2_i_b_split004_tail2.c \
	src/overlays/field/func_8008AABC.c \
	src/overlays/field/func_8008AD44.c \
	src/overlays/field/field_queue_actor_animation_by_handle.c \
	src/overlays/field/unk2_i_b_split005.c \
	src/overlays/field/func_8008C730.c \
	src/overlays/field/unk2_i_b_split006_tail2.c \
	src/overlays/field/unk2_i_b_split006_tail.c \
	src/overlays/field/func_8008EF0C.c \
	src/overlays/field/unk2_i_b_split006_tail_after_8EF0C.c \
	src/overlays/field/func_8008D104.c \
	src/overlays/field/func_80090F80.c \
	src/overlays/field/unk2_i_b_split007_tail.c \
	src/overlays/field/unk2_i_b_split008.c \
	src/overlays/field/func_8009184C.c \
	src/overlays/field/unk2_i_b_split008_after_9184C.c \
	src/overlays/field/func_800920FC.c \
	src/overlays/field/unk2_i_b_split008_tail.c \
	src/overlays/field/func_80092200.c \
	src/overlays/field/func_80092394.c \
	src/overlays/field/func_800923F0.c \
	src/overlays/field/unk2_i_b_split008_tail2_tail.c \
	src/overlays/field/unk2_i_b_split008_tail2b.c \
	src/overlays/field/func_80092C24.c \
	src/overlays/field/func_80094690.c \
	src/overlays/field/unk2_i_b_split008_tail3.c \
	src/overlays/field/func_80094B5C.c \
	src/overlays/field/unk2_i_b_split009.c \
	src/overlays/field/func_80094FDC.c \
	src/overlays/field/func_80095074.c \
	src/overlays/field/func_80095168.c \
	src/overlays/field/unk2_i_b_split009_tail.c \
	src/overlays/field/func_8009A3E8.c \
	src/overlays/field/unk2_i_b_split011.c \
	src/overlays/field/unk2_i_b_split012.c \
	src/overlays/field/unk2_i_b_split013.c \
	src/overlays/field/func_8009C4B4.c \
	src/overlays/field/unk2_i_b_split013_b.c \
	src/overlays/field/unk2_i_b_split014.c \
	src/overlays/field/unk2_i_b_split016.c \
	src/overlays/field/unk2_i_b_split016_b.c \
	src/overlays/field/func_8009CF1C.c \
	src/overlays/field/unk2_i_b_split016_tail2.c \
	src/overlays/field/unk2_i_b_split016_tail_b.c \
	src/overlays/field/func_8009E66C.c \
	src/overlays/field/unk2_i_b_split016_tail_b_tail.c \
	src/overlays/field/unk2_i_b_split017.c \
	src/overlays/field/func_800A2990.c \
	src/overlays/field/unk2_i_b_split018.c \
	src/overlays/field/unk2_i_b_split019.c \
	src/overlays/field/field_restore_entry_music.c \
	src/overlays/field/func_800A39A8.c \
	src/overlays/field/unk2_b_split001.c \
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
	src/overlays/field/unk2_b_split005.c \
	src/overlays/field/unk2_b_split006.c \
	src/overlays/field/unk2_b_split007.c \
	src/overlays/field/unk2_b_split007_b.c \
	src/overlays/field/unk2_b_split008.c \
	src/overlays/field/unk2_b_split008_b.c \
	src/overlays/field/func_800A7384.c \
	src/overlays/field/unk2_b_split009.c \
	src/overlays/field/func_800A88A0.c \
	src/overlays/field/func_800A90F8.c \
	src/overlays/field/unk2_b_split013_tail_a.c \
	src/overlays/field/unk2_b_split013_b.c \
	src/overlays/field/func_800AA02C.c \
	src/overlays/field/unk2_b_split013_tail.c \
	src/overlays/field/unk2_b_split014.c \
	src/overlays/field/unk2_b_split015.c \
	src/overlays/field/func_800AD120.c \
	src/overlays/field/func_800AD7DC.c \
	src/overlays/field/unk2_b_split015_tail.c \
	src/overlays/field/func_800ADF84.c \
	src/overlays/field/unk2_b_split016.c \
	src/overlays/field/unk2_b_split017.c \
	src/overlays/field/func_800AE9E0.c \
	src/overlays/field/unk2_b_split017_tail.c \
	src/overlays/field/unk2_b_split018.c \
	src/overlays/field/func_800AF8E8.c \
	src/overlays/field/unk2_b_split019.c \
	src/overlays/field/unk2_b_split020.c \
	src/overlays/field/func_800B0888.c \
	src/overlays/field/unk2_b_split021_tail.c \
	src/overlays/field/unk2_b_split022.c \
	src/overlays/field/unk2_b_split022_b.c \
	src/overlays/field/unk2_b_split022_b_tail.c \
	src/overlays/field/unk2_b_split023_tail.c \
	src/overlays/field/unk2_b_split025_tail2_c.c \
	src/overlays/field/unk2_b_split025_tail2b.c \
	src/overlays/field/unk2_b_split025_tail2b_b.c \
	src/overlays/field/unk2_b_split025_tail_b.c \
	src/overlays/field/unk2_b_split026.c \
	src/overlays/field/unk2_b_split026_tail.c \
	src/overlays/field/unk2_b_split028.c \
	src/overlays/field/unk2_b_split029_mid.c \
	src/overlays/field/func_800B7A74.c \
	src/overlays/field/unk2_b_split030.c \
	src/overlays/field/unk2_b_split031_after_89d0.c \
	src/overlays/field/unk2_b_split031_mid.c \
	src/overlays/field/unk2_b_split031_tail.c \
	src/overlays/field/unk2_b_split034.c \
	src/overlays/field/func_800BCC74.c \
	src/overlays/field/unk2_b_split040.c \
	src/overlays/field/unk2_b_split046.c \
	src/overlays/field/unk2_b_split048.c \
	src/overlays/field/unk2_b_split049.c \
	src/overlays/field/unk2_b_split051.c \
	src/overlays/field/unk2_b_split051_b.c \
	src/overlays/field/unk2_b_split051_c.c \
	src/overlays/field/unk2_b_split052.c \
	src/overlays/field/unk2_b_split053.c \
	src/overlays/field/unk2_b_split053_b.c \
	src/overlays/field/unk2_b_split055.c \
	src/overlays/field/func_800C10F0.c \
	src/overlays/field/unk2_b_split056.c \
	src/overlays/field/unk2_b_split056_b.c \
	src/overlays/field/unk2_b_split057.c \
	src/overlays/field/unk2_b_split062.c \
	src/overlays/field/unk2_b_split062_after_23f4.c \
	src/overlays/field/unk2_b_split062_b.c \
	src/overlays/field/unk2_b_split064.c \
	src/overlays/field/unk2_b_split064_tail.c \
	src/overlays/field/func_800C3688.c \
	src/overlays/field/unk2_b_split067_b.c \
	src/overlays/field/unk2_b_split068.c \
	src/overlays/field/func_800C3CB4.c \
	src/overlays/field/unk2_b_split068_tail.c \
	src/overlays/field/func_800C4364.c \
	src/overlays/field/unk2_b_split069.c \
	src/overlays/field/unk2_b_split070_tail.c \
	src/overlays/field/unk2_b_split071.c \
	src/overlays/field/unk2_b_split073_tail.c \
	src/overlays/field/unk2_b_split075.c \
	src/overlays/field/func_800C6A30.c \
	src/overlays/field/func_800C6EBC.c \
	src/overlays/field/unk2_b_split078.c \
	src/overlays/field/unk2_b_split081.c \
	src/overlays/field/func_800C7CF8.c \
	src/overlays/field/func_800C7F44.c \
	src/overlays/field/unk2_b_split081_tail.c \
	src/overlays/field/unk2_b_split081_tail_b.c \
	src/overlays/field/unk2_b_split082.c \
	src/overlays/field/unk2_b_split083.c \
	src/overlays/field/unk2_b_split085.c \
	src/overlays/field/unk2_b_split086.c \
	src/overlays/field/unk2_b_split087_tail.c \
	src/overlays/field/unk2_b_split088.c \
	src/overlays/field/unk2_b_split098.c \
	src/overlays/field/unk2_b_split098_tail.c \
	src/overlays/field/field235.c \
	src/overlays/field/func_800A8E28.c \
	src/overlays/field/unk2_b_split100.c \
	src/overlays/field/func_80087614.c \
	src/overlays/field/unk2_i_b_split020_tail.c \
	src/overlays/field/field242.c \
	src/overlays/field/unk2_i_b_split021.c \
	src/overlays/field/func_800A20DC.c \
	src/overlays/field/unk2_i_b_split021_tail.c \
	src/overlays/field/field244.c \
	src/overlays/field/func_800AB070.c \
	src/overlays/field/unk2_b_split105_b.c \
	src/overlays/field/unk2_b_split106_tail_b.c \
	src/overlays/field/unk2_b_split106_tail_b_after_6b28.c \
	src/overlays/field/unk2_b_split107_tail_b_after_9ea8.c \
	src/overlays/field/unk2_b_split111.c \
	src/overlays/field/unk2_b_split112.c \
	src/overlays/field/unk2_b_split113.c \
	src/overlays/field/field_handle_actor_control_flag_40.c \
	src/overlays/field/func_80086ACC.c \
	src/overlays/field/func_80086C70.c \
	src/overlays/field/func_80086C00.c \
	src/overlays/field/func_80086D5C.c \
	src/overlays/field/unk2_b_split115.c \
	src/overlays/field/unk2_b_split117.c \
	src/overlays/field/unk2_b_split118.c \
	src/overlays/field/unk2_b_split119.c \
	src/overlays/field/unk2_b_split120.c \
	src/overlays/field/func_800BF9A0.c \
	src/overlays/field/unk2_b_split124.c \
	src/overlays/field/unk2_b_split125.c \
	src/overlays/field/func_8008AEB0.c \
	src/overlays/field/unk2_i_b_split022_tail.c \
	src/overlays/field/func_8008B1C8.c \
	src/overlays/field/unk2_i_b_split022_tail_after_8B1C8.c \
	src/overlays/field/func_8008AF68.c \
	src/overlays/field/func_8008B398.c \
	src/overlays/field/func_8008B42C.c \
	src/overlays/field/func_8008B500.c \
	src/overlays/field/unk2_i_b_split022_tail_b.c \
	src/overlays/field/unk2_i_b_split023.c \
	src/overlays/field/func_80098FC4.c \
	src/overlays/field/unk2_i_b_split023_tail.c \
	src/overlays/field/func_8009A2A4.c \
	src/overlays/field/field_set_actor_horizontal_scale.c \
	src/overlays/field/field254.c \
	src/overlays/field/field255.c \
	src/overlays/field/field256.c \
	src/overlays/field/field257.c \
	src/overlays/field/field258.c \
	src/overlays/field/field262.c \
	src/overlays/field/func_800BF880.c \
	src/overlays/field/func_800C1EC8.c \
	src/overlays/field/field264.c \
	src/overlays/field/field265.c \
	src/overlays/field/field268.c \
	src/overlays/field/field269.c \
	src/overlays/field/field273.c \
	src/overlays/field/field_load_selected_equipment_mystic_cards.c \
	src/overlays/field/field_upload_initial_vram_resource.c \
	src/overlays/field/unk2_i_b_split024_tail.c \
	src/overlays/field/field288.c \
	src/overlays/field/field289.c \
	src/overlays/field/field290.c \
	src/overlays/field/field297.c \
	src/overlays/field/field298.c \
	src/overlays/field/unk2_b_split010_tail.c \
	src/overlays/field/func_800B7B98.c \
	src/overlays/field/unk2_b_split029_tail.c \
	src/overlays/field/unk2_b_split029_tail_after_820c.c \
	src/overlays/field/func_8008BD88.c \
	src/overlays/field/unk2_i_b_split005_tail.c \
	src/overlays/field/func_8008C024.c \
	src/overlays/field/unk2_i_b_split005_tail_b.c \
	src/overlays/field/func_80096A90.c \
	src/overlays/field/unk2_i_b_split023_mid_after_96A90.c \
	src/overlays/field/func_800970B0.c \
	src/overlays/field/unk2_i_b_split023_mid_tail.c \
	src/overlays/field/unk2_i_b_split023_mid_b.c \
	src/overlays/field/unk2_b_split025_tail3.c \
	src/overlays/field/unk2_b_split127.c \
	src/overlays/field/func_800C7168.c

overlay_field_gcc_272_cdk_g0_nosched_srcs := src/overlays/field/field2.c
overlay_field_gcc_272_cdk_g0_noexpand_srcs := src/overlays/field/field_text_format_number.c
overlay_field_gcc_280_g0_srcs := \
	src/overlays/field/field_menu_record_ops.c \
	src/overlays/field/unk2_b_split025_tail.c \
	src/overlays/field/unk2_b_split082_b.c \
	src/overlays/field/func_800B622C.c \
	src/overlays/field/func_800BD318.c \
	src/overlays/field/func_800BDBAC.c \
	src/overlays/field/func_800C8FA8.c \
	src/overlays/field/field303.c \
	src/overlays/field/field305.c \
	src/overlays/field/field309.c \
	src/overlays/field/field311.c \
	src/overlays/field/field325.c \
	src/overlays/field/field338.c \
	src/overlays/field/field339.c \
	src/overlays/field/field343.c \
	src/overlays/field/field344.c \
	src/overlays/field/field345.c \
	src/overlays/field/field346.c \
	src/overlays/field/field348.c \
	src/overlays/field/func_800C38C8.c \
	src/overlays/field/func_800C23F4.c \
	src/overlays/field/func_800C6C80.c \
	src/overlays/field/field351.c \
	src/overlays/field/field353.c \
	src/overlays/field/field354.c \
	src/overlays/field/func_800B820C.c \
	src/overlays/field/field371.c \
	src/overlays/field/field373.c \
	src/overlays/field/field356.c \
	src/overlays/field/field357.c \
	src/overlays/field/field358.c \
	src/overlays/field/field360.c \
	src/overlays/field/field361.c \
	src/overlays/field/field362.c \
	src/overlays/field/field363.c \
	src/overlays/field/field364.c \
	src/overlays/field/field365.c \
	src/overlays/field/field366.c \
	src/overlays/field/field368.c \
	src/overlays/field/field369.c \
	src/overlays/field/field370.c \
	src/overlays/field/field_equipment_combination_rules.c \
	src/overlays/field/field87.c \
	src/overlays/field/field89.c \
	src/overlays/field/func_800B31CC.c \
	src/overlays/field/func_800B32FC.c \
	src/overlays/field/func_800B3420.c \
	src/overlays/field/func_800B34D0.c \
	src/overlays/field/func_800B3580.c \
	src/overlays/field/func_800B3670.c \
	src/overlays/field/func_800B3D84.c \
	src/overlays/field/func_800B4844.c \
	src/overlays/field/func_800B4CE4.c \
	src/overlays/field/func_800B4DF0.c \
	src/overlays/field/func_800B607C.c \
	src/overlays/field/func_800B661C.c \
	src/overlays/field/func_800B6744.c \
	src/overlays/field/func_800B70F4.c \
	src/overlays/field/func_800B7020.c \
	src/overlays/field/func_800C8964.c \
	src/overlays/field/func_800B78C0.c \
	src/overlays/field/func_800BC474.c \
	src/overlays/field/func_800BCAD8.c \
	src/overlays/field/func_800BCE94.c \
	src/overlays/field/func_800BCEFC.c \
	src/overlays/field/func_800BD434.c \
	src/overlays/field/func_800BD4A8.c \
	src/overlays/field/func_800BD55C.c \
	src/overlays/field/func_800BD650.c \
	src/overlays/field/func_800BE550.c \
	src/overlays/field/func_800BF68C.c \
	src/overlays/field/func_800C1B98.c \
	src/overlays/field/func_800C21C0.c \
	src/overlays/field/func_800C2848.c \
	src/overlays/field/func_800C29CC.c \
	src/overlays/field/func_800C2D08.c \
	src/overlays/field/func_800C32C8.c \
	src/overlays/field/func_800C33E4.c \
	src/overlays/field/func_800C36F0.c \
	src/overlays/field/func_800C5760.c \
	src/overlays/field/func_800C6228.c \
	src/overlays/field/func_800C7494.c \
	src/overlays/field/func_800BD3B0.c \
	src/overlays/field/field_golem_logic_blocks.c \
	src/overlays/field/field_script_ops_00_01.c \
	src/overlays/field/field_script_ops_03_08.c \
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
	src/overlays/field/field102.c \
	src/overlays/field/field103.c \
	src/overlays/field/field105.c \
	src/overlays/field/field109.c \
	src/overlays/field/field116.c \
	src/overlays/field/field119.c \
	src/overlays/field/field124.c \
	src/overlays/field/field125.c \
	src/overlays/field/func_800BD6F4.c \
	src/overlays/field/field126.c \
	src/overlays/field/field127.c \
	src/overlays/field/field129.c \
	src/overlays/field/field130.c \
	src/overlays/field/field131.c \
	src/overlays/field/func_800BE680.c \
	src/overlays/field/field143.c \
	src/overlays/field/field145.c \
	src/overlays/field/field148.c \
	src/overlays/field/func_800C15AC.c \
	src/overlays/field/field149.c \
	src/overlays/field/field150.c \
	src/overlays/field/field152.c \
	src/overlays/field/field154.c \
	src/overlays/field/field155.c \
	src/overlays/field/field156.c \
	src/overlays/field/field159.c \
	src/overlays/field/field161.c \
	src/overlays/field/field163.c \
	src/overlays/field/field166.c \
	src/overlays/field/field167.c \
	src/overlays/field/field169.c \
	src/overlays/field/field170.c \
	src/overlays/field/func_800C5B64.c \
	src/overlays/field/field173.c \
	src/overlays/field/field174.c \
	src/overlays/field/field176.c \
	src/overlays/field/field179.c \
	src/overlays/field/field180.c \
	src/overlays/field/field181.c \
	src/overlays/field/field184.c \
	src/overlays/field/field186.c \
	src/overlays/field/field188.c \
	src/overlays/field/func_800C7558.c \
	src/overlays/field/func_800C7C88.c \
	src/overlays/field/func_800C7D5C.c \
	src/overlays/field/func_800C8014.c \
	src/overlays/field/field190.c \
	src/overlays/field/field191.c \
	src/overlays/field/field193.c \
	src/overlays/field/field197.c \
	src/overlays/field/field199.c \
	src/overlays/field/field236.c \
	src/overlays/field/func_800B19FC.c \
	src/overlays/field/func_800B286C.c \
	src/overlays/field/func_800B2A9C.c \
	src/overlays/field/func_800B3160.c \
	src/overlays/field/func_800B6890.c \
	src/overlays/field/func_800B6B28.c \
	src/overlays/field/field243.c \
	src/overlays/field/field245.c \
	src/overlays/field/func_800B66F0.c \
	src/overlays/field/func_800B8CFC.c \
	src/overlays/field/field252.c \
	src/overlays/field/field259.c \
	src/overlays/field/field260.c \
	src/overlays/field/field261.c \
	src/overlays/field/field263.c \
	src/overlays/field/field266.c \
	src/overlays/field/func_800C1D68.c \
	src/overlays/field/field267.c \
	src/overlays/field/field270.c \
	src/overlays/field/field271.c \
	src/overlays/field/field274.c \
	src/overlays/field/field276.c \
	src/overlays/field/field277.c \
	src/overlays/field/field278.c \
	src/overlays/field/field280.c \
	src/overlays/field/field281.c \
	src/overlays/field/field283.c \
	src/overlays/field/field284.c \
	src/overlays/field/field285.c \
	src/overlays/field/field286.c \
	src/overlays/field/field287.c \
	src/overlays/field/field291.c \
	src/overlays/field/field292.c \
	src/overlays/field/field293.c \
	src/overlays/field/field295.c \
	src/overlays/field/field296.c \
	src/overlays/field/field299.c \
	src/overlays/field/field300.c \
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
overlay_gname_gcc_272_cdk_g0_srcs := src/overlays/gname/gname.c

OVERLAYS += golem
overlay_golem_gcc_272_cdk_g0_srcs := src/overlays/golem/golem.c

OVERLAYS += gosub
overlay_gosub_gcc_272_cdk_g0_srcs := src/overlays/gosub/gosub.c

OVERLAYS += gover
overlay_gover_gcc_272_cdk_g0_srcs := src/overlays/gover/gover.c

OVERLAYS += menu
overlay_menu_gcc_272_cdk_g0_srcs := src/overlays/menu/menu_rodata.c src/overlays/menu/menu.c

OVERLAYS += movie
overlay_movie_gcc_280_g4_srcs := src/overlays/movie/movie.c

OVERLAYS += niki
overlay_niki_gcc_272_cdk_g0_srcs := src/overlays/niki/niki.c

OVERLAYS += shop
overlay_shop_gcc_272_cdk_g0_srcs := \
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
overlay_title_gcc_272_cdk_g0_srcs := src/overlays/title/title.c

OVERLAYS += wsel
overlay_wsel_gcc_272_cdk_g0_srcs := \
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
overlay_zukan_gcc_272_cdk_g0_srcs := \
	src/overlays/zukan/zukan_init.c \
	src/overlays/zukan/zukan_build_entry_primitives.c \
	src/overlays/zukan/zukan_image_upload.c \
	src/overlays/zukan/zukan_update_frame.c \
	src/overlays/zukan/func_80141354.c \
	src/overlays/zukan/zukan_mode_setters.c \
	src/overlays/zukan/zukan_transition_update.c \
	src/overlays/zukan/func_80141DF4.c \
	src/overlays/zukan/zukan_resource_helpers.c \
	src/overlays/zukan/zukan_resource_table_init.c \
	src/overlays/zukan/zukan_resource_text.c \
	src/overlays/zukan/zukan_resource_sprites.c \
	src/overlays/zukan/zukan_resource_loader.c \
	src/overlays/zukan/func_80142CA0.c
overlay_zukan_gcc_280_g0_srcs := \
	src/overlays/zukan/zukan_scroll_window.c \
	src/overlays/zukan/zukan_gpu_modes.c \
	src/overlays/zukan/func_80141988.c \
	src/overlays/zukan/zukan_outline_fade.c \
	src/overlays/zukan/unk1_tail_after_resource_helpers.c
