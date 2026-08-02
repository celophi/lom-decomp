# ============================================================================
# Overlay registry and per-source toolchain routing
# ============================================================================

# ─── Overlay Registry ──────────────────────────────────────────────────────────
#
# Register overlays here so they're available to all rules below.
# The name must match the directory under src/overlays/, asm/overlays/, etc.
# See the "Overlay System" section further below for full documentation.
#
# Optional per-overlay settings:
#   overlay_<name>_cflags    — compiler flags (default: CFLAGS_G0)
#   overlay_<name>_asset     — path to a .bin asset file (omit if none)
#   overlay_<name>_gcc_srcs  — files to compile with GCC+maspsx instead of CDK gcc
#                              (use for non-matching stubs that use INCLUDE_ASM)

OVERLAYS += addhero
overlay_addhero_gcc_srcs   := src/overlays/addhero/unk1.c

OVERLAYS += carda
overlay_carda_gcc_srcs   := src/overlays/carda/unk1.c

OVERLAYS += checkps
overlay_checkps_gcc_srcs   := src/overlays/checkps/code3.c
overlay_checkps_gnu_srcs   := src/overlays/checkps/code4.c src/overlays/checkps/code6.c src/overlays/checkps/code7.c

OVERLAYS += cload
overlay_cload_gcc_srcs   := src/overlays/cload/unk1.c

OVERLAYS += field
overlay_field_gcc_srcs      := src/overlays/field/unk2.c
overlay_field_gcc_g4_noexpand_srcs := src/overlays/field/field_scene_load.c src/overlays/field/field_scene_build.c src/overlays/field/field_render.c src/overlays/field/field_animation.c src/overlays/field/field_scene_api.c src/overlays/field/field_collision.c src/overlays/field/field5.c src/overlays/field/func_800674a8.c src/overlays/field/func_80067598.c src/overlays/field/func_80067b8c.c src/overlays/field/func_8006429c.c

OVERLAYS += gname

OVERLAYS += golem
overlay_golem_gcc_srcs   := src/overlays/golem/unk1.c

OVERLAYS += gosub
overlay_gosub_gcc_srcs   := src/overlays/gosub/unk1.c

OVERLAYS += gover

OVERLAYS += menu

OVERLAYS += movie
overlay_movie_gcc_g4_srcs   := src/overlays/movie/movie.c

OVERLAYS += niki
overlay_niki_gcc_srcs   := src/overlays/niki/unk1.c

OVERLAYS += shop
overlay_shop_gcc_srcs   := src/overlays/shop/unk1.c

OVERLAYS += title

OVERLAYS += wsel
overlay_wsel_gcc_srcs    := src/overlays/wsel/unk1.c

OVERLAYS += zukan
overlay_zukan_gcc_srcs    := src/overlays/zukan/unk1.c
