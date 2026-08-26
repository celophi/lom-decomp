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
overlay_addhero_gcc_280_g0_srcs := src/overlays/addhero/unk1.c

OVERLAYS += carda
overlay_carda_gcc_280_g0_srcs := src/overlays/carda/unk1.c

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
	src/overlays/field/field4.c \
	src/overlays/field/field_audio.c \
	src/overlays/field/func_800681c0.c \
	src/overlays/field/func_800675c8.c \
	src/overlays/field/func_80067aa4.c \
	src/overlays/field/func_80067fb0.c \
	src/overlays/field/field6.c \
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
	src/overlays/field/field18.c \
	src/overlays/field/field19.c \
	src/overlays/field/field20.c \
	src/overlays/field/field21.c \
	src/overlays/field/field22.c \
	src/overlays/field/field23.c \
	src/overlays/field/field24.c \
	src/overlays/field/field25.c \
	src/overlays/field/field26.c \
	src/overlays/field/field27.c \
	src/overlays/field/field28.c \
	src/overlays/field/field29.c \
	src/overlays/field/unk2.c \
	src/overlays/field/unk2_c.c \
	src/overlays/field/unk2_d.c \
	src/overlays/field/unk2_e.c \
	src/overlays/field/unk2_f.c \
	src/overlays/field/unk2_f_b.c \
	src/overlays/field/unk2_g.c \
	src/overlays/field/unk2_h.c \
	src/overlays/field/unk2_h_b.c \
	src/overlays/field/unk2_i.c \
	src/overlays/field/unk2_i_b.c \
	src/overlays/field/unk2_b.c
overlay_field_gcc_272_cdk_g0_nosched_srcs := src/overlays/field/field2.c
overlay_field_gcc_272_cdk_g0_noexpand_srcs := src/overlays/field/field3.c
overlay_field_gcc_280_g4_srcs := src/overlays/field/func_80067bbc.c
overlay_field_gcc_280_g4_noexpand_srcs := \
	src/overlays/field/field_scene_load.c \
	src/overlays/field/field_scene_build.c \
	src/overlays/field/field_render.c \
	src/overlays/field/field_animation.c \
	src/overlays/field/field_scene_api.c \
	src/overlays/field/field_collision.c \
	src/overlays/field/field5.c \
	src/overlays/field/func_800674a8.c \
	src/overlays/field/func_80067598.c \
	src/overlays/field/func_80067b8c.c \
	src/overlays/field/field1.c

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
overlay_niki_gcc_280_g0_srcs := src/overlays/niki/unk1.c
overlay_niki_gcc_272_cdk_g0_srcs := src/overlays/niki/niki.c

OVERLAYS += shop
overlay_shop_gcc_280_g0_srcs := src/overlays/shop/unk1.c

OVERLAYS += title
overlay_title_gcc_272_cdk_g0_srcs := src/overlays/title/title.c

OVERLAYS += wsel
overlay_wsel_gcc_280_g0_srcs := src/overlays/wsel/unk1.c

OVERLAYS += zukan
overlay_zukan_gcc_280_g0_srcs := src/overlays/zukan/unk1.c
