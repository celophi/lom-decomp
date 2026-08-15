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
	src/overlays/checkps/code.c \
	src/overlays/checkps/code2.c
overlay_checkps_gcc_280_g0_srcs := src/overlays/checkps/code3.c
overlay_checkps_gcc_272_gnu_g0_srcs := \
	src/overlays/checkps/code4.c \
	src/overlays/checkps/code6.c \
	src/overlays/checkps/code7.c

OVERLAYS += cload
overlay_cload_gcc_280_g0_srcs := src/overlays/cload/unk1.c

OVERLAYS += field
overlay_field_gcc_272_cdk_g0_srcs := \
	src/overlays/field/field4.c \
	src/overlays/field/field_audio.c \
	src/overlays/field/func_800681c0.c \
	src/overlays/field/unk1_c_b.c \
	src/overlays/field/unk1_c_tail.c \
	src/overlays/field/unk1_c_tail2.c \
	src/overlays/field/unk1_c_tail3.c \
	src/overlays/field/unk1_f.c \
	src/overlays/field/unk1_g.c \
	src/overlays/field/unk2_b.c
overlay_field_gcc_280_g0_srcs := src/overlays/field/unk2.c
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
overlay_gname_gcc_272_cdk_g0_srcs := src/overlays/gname/gname.c src/overlays/gname/gname_data.c

OVERLAYS += golem
overlay_golem_gcc_280_g0_srcs := src/overlays/golem/unk1.c

OVERLAYS += gosub
overlay_gosub_gcc_272_cdk_g0_srcs := src/overlays/gosub/gosub.c
overlay_gosub_gcc_280_g0_srcs := src/overlays/gosub/unk1_b.c

OVERLAYS += gover
overlay_gover_gcc_272_cdk_g0_srcs := src/overlays/gover/gover.c

OVERLAYS += menu
overlay_menu_gcc_272_cdk_g0_srcs := \
	src/overlays/menu/menu.c \
	src/overlays/menu/menu2.c

OVERLAYS += movie
overlay_movie_gcc_280_g4_srcs := src/overlays/movie/movie.c

OVERLAYS += niki
overlay_niki_gcc_280_g0_srcs := src/overlays/niki/unk1.c

OVERLAYS += shop
overlay_shop_gcc_280_g0_srcs := src/overlays/shop/unk1.c

OVERLAYS += title
overlay_title_gcc_272_cdk_g0_srcs := src/overlays/title/title.c

OVERLAYS += wsel
overlay_wsel_gcc_280_g0_srcs := src/overlays/wsel/unk1.c

OVERLAYS += zukan
overlay_zukan_gcc_280_g0_srcs := src/overlays/zukan/unk1.c
