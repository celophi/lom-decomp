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
	src/overlays/field/unk2_e.c \
	src/overlays/field/unk2_f.c \
	src/overlays/field/unk2_f_b.c \
	src/overlays/field/unk2_g.c \
	src/overlays/field/unk2_h.c \
	src/overlays/field/unk2_h_b.c \
	src/overlays/field/unk2_i.c \
	src/overlays/field/unk2_b.c \
	src/overlays/field/field30.c \
	src/overlays/field/field31.c \
	src/overlays/field/field32.c \
	src/overlays/field/field33.c \
	src/overlays/field/field34.c \
	src/overlays/field/field35.c \
	src/overlays/field/field36.c \
	src/overlays/field/field37.c \
	src/overlays/field/field38.c \
	src/overlays/field/field39.c \
	src/overlays/field/field40.c \
	src/overlays/field/field41.c \
	src/overlays/field/field42.c \
	src/overlays/field/field43.c \
	src/overlays/field/field44.c \
	src/overlays/field/field45.c \
	src/overlays/field/field46.c \
	src/overlays/field/field47.c \
	src/overlays/field/field48.c \
	src/overlays/field/field49.c \
	src/overlays/field/field50.c \
	src/overlays/field/field51.c \
	src/overlays/field/field52.c \
	src/overlays/field/field53.c \
	src/overlays/field/field54.c \
	src/overlays/field/field55.c \
	src/overlays/field/field56.c \
	src/overlays/field/field57.c \
	src/overlays/field/field58.c \
	src/overlays/field/field59.c \
	src/overlays/field/field60.c \
	src/overlays/field/field61.c \
	src/overlays/field/field62.c \
	src/overlays/field/field63.c \
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
	src/overlays/field/field77.c \
	src/overlays/field/field78.c \
	src/overlays/field/field79.c \
	src/overlays/field/field80.c \
	src/overlays/field/field81.c \
	src/overlays/field/field82.c \
	src/overlays/field/field83.c \
	src/overlays/field/field84.c \
	src/overlays/field/field85.c \
	src/overlays/field/field86.c \
	src/overlays/field/field88.c \
	src/overlays/field/field90.c \
	src/overlays/field/field93.c \
	src/overlays/field/field94.c \
	src/overlays/field/field96.c \
	src/overlays/field/field97.c \
	src/overlays/field/field98.c \
	src/overlays/field/field108.c \
	src/overlays/field/field111.c \
	src/overlays/field/field112.c \
	src/overlays/field/field113.c \
	src/overlays/field/field114.c \
	src/overlays/field/field115.c \
	src/overlays/field/field118.c \
	src/overlays/field/field120.c \
	src/overlays/field/field121.c \
	src/overlays/field/field122.c \
	src/overlays/field/field123.c \
	src/overlays/field/field132.c \
	src/overlays/field/field133.c \
	src/overlays/field/field134.c \
	src/overlays/field/field135.c \
	src/overlays/field/field136.c \
	src/overlays/field/field137.c \
	src/overlays/field/field138.c \
	src/overlays/field/field139.c \
	src/overlays/field/field140.c \
	src/overlays/field/field141.c \
	src/overlays/field/field142.c \
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
	src/overlays/field/field189.c \
	src/overlays/field/field192.c \
	src/overlays/field/field194.c \
	src/overlays/field/field195.c \
	src/overlays/field/field196.c \
	src/overlays/field/field198.c \
	src/overlays/field/field200.c \
	src/overlays/field/field202.c \
	src/overlays/field/field203.c \
	src/overlays/field/field204.c \
	src/overlays/field/field205.c \
	src/overlays/field/field206.c \
	src/overlays/field/field207.c \
	src/overlays/field/field209.c \
	src/overlays/field/field216.c \
	src/overlays/field/field229.c \
	src/overlays/field/field230.c \
	src/overlays/field/field231.c \
	src/overlays/field/field232.c \
	src/overlays/field/field233.c \
	src/overlays/field/field234.c \
	src/overlays/field/unk2_split001.c \
	src/overlays/field/unk2_i_b_split002.c \
	src/overlays/field/unk2_i_b_split003.c \
	src/overlays/field/unk2_i_b_split004.c \
	src/overlays/field/unk2_i_b_split005.c \
	src/overlays/field/unk2_i_b_split006.c \
	src/overlays/field/unk2_i_b_split007.c \
	src/overlays/field/unk2_i_b_split008.c \
	src/overlays/field/unk2_i_b_split009.c \
	src/overlays/field/unk2_i_b_split010.c \
	src/overlays/field/unk2_i_b_split011.c \
	src/overlays/field/unk2_i_b_split012.c \
	src/overlays/field/unk2_i_b_split013.c \
	src/overlays/field/unk2_i_b_split014.c \
	src/overlays/field/unk2_i_b_split015.c \
	src/overlays/field/unk2_i_b_split016.c \
	src/overlays/field/unk2_i_b_split017.c \
	src/overlays/field/unk2_i_b_split018.c \
	src/overlays/field/unk2_i_b_split019.c \
	src/overlays/field/unk2_b_split001.c \
	src/overlays/field/unk2_b_split002.c \
	src/overlays/field/unk2_b_split003.c \
	src/overlays/field/unk2_b_split004.c \
	src/overlays/field/unk2_b_split005.c \
	src/overlays/field/unk2_b_split006.c \
	src/overlays/field/unk2_b_split007.c \
	src/overlays/field/unk2_b_split008.c \
	src/overlays/field/unk2_b_split009.c \
	src/overlays/field/unk2_b_split010.c \
	src/overlays/field/unk2_b_split011.c \
	src/overlays/field/unk2_b_split012.c \
	src/overlays/field/unk2_b_split013.c \
	src/overlays/field/unk2_b_split014.c \
	src/overlays/field/unk2_b_split015.c \
	src/overlays/field/unk2_b_split016.c \
	src/overlays/field/unk2_b_split017.c \
	src/overlays/field/unk2_b_split018.c \
	src/overlays/field/unk2_b_split019.c \
	src/overlays/field/unk2_b_split020.c \
	src/overlays/field/unk2_b_split021.c \
	src/overlays/field/unk2_b_split022.c \
	src/overlays/field/unk2_b_split023.c \
	src/overlays/field/unk2_b_split024.c \
	src/overlays/field/unk2_b_split025.c \
	src/overlays/field/unk2_b_split026.c \
	src/overlays/field/unk2_b_split027.c \
	src/overlays/field/unk2_b_split028.c \
	src/overlays/field/unk2_b_split029.c \
	src/overlays/field/unk2_b_split030.c \
	src/overlays/field/unk2_b_split031.c \
	src/overlays/field/unk2_b_split032.c \
	src/overlays/field/unk2_b_split033.c \
	src/overlays/field/unk2_b_split034.c \
	src/overlays/field/unk2_b_split035.c \
	src/overlays/field/unk2_b_split036.c \
	src/overlays/field/unk2_b_split037.c \
	src/overlays/field/unk2_b_split038.c \
	src/overlays/field/unk2_b_split039.c \
	src/overlays/field/unk2_b_split040.c \
	src/overlays/field/unk2_b_split041.c \
	src/overlays/field/unk2_b_split042.c \
	src/overlays/field/unk2_b_split043.c \
	src/overlays/field/unk2_b_split044.c \
	src/overlays/field/unk2_b_split045.c \
	src/overlays/field/unk2_b_split046.c \
	src/overlays/field/unk2_b_split047.c \
	src/overlays/field/unk2_b_split048.c \
	src/overlays/field/unk2_b_split049.c \
	src/overlays/field/unk2_b_split050.c \
	src/overlays/field/unk2_b_split051.c \
	src/overlays/field/unk2_b_split052.c \
	src/overlays/field/unk2_b_split053.c \
	src/overlays/field/unk2_b_split054.c \
	src/overlays/field/unk2_b_split055.c \
	src/overlays/field/unk2_b_split056.c \
	src/overlays/field/unk2_b_split057.c \
	src/overlays/field/unk2_b_split058.c \
	src/overlays/field/unk2_b_split059.c \
	src/overlays/field/unk2_b_split060.c \
	src/overlays/field/unk2_b_split061.c \
	src/overlays/field/unk2_b_split062.c \
	src/overlays/field/unk2_b_split063.c \
	src/overlays/field/unk2_b_split064.c \
	src/overlays/field/unk2_b_split065.c \
	src/overlays/field/unk2_b_split066.c \
	src/overlays/field/unk2_b_split067.c \
	src/overlays/field/unk2_b_split068.c \
	src/overlays/field/unk2_b_split069.c \
	src/overlays/field/unk2_b_split070.c \
	src/overlays/field/unk2_b_split071.c \
	src/overlays/field/unk2_b_split072.c \
	src/overlays/field/unk2_b_split073.c \
	src/overlays/field/unk2_b_split074.c \
	src/overlays/field/unk2_b_split075.c \
	src/overlays/field/unk2_b_split076.c \
	src/overlays/field/unk2_b_split077.c \
	src/overlays/field/unk2_b_split078.c \
	src/overlays/field/unk2_b_split079.c \
	src/overlays/field/unk2_b_split080.c \
	src/overlays/field/unk2_b_split081.c \
	src/overlays/field/unk2_b_split082.c \
	src/overlays/field/unk2_b_split083.c \
	src/overlays/field/unk2_b_split084.c \
	src/overlays/field/unk2_b_split085.c \
	src/overlays/field/unk2_b_split086.c \
	src/overlays/field/unk2_b_split087.c \
	src/overlays/field/unk2_b_split088.c \
	src/overlays/field/unk2_b_split089.c \
	src/overlays/field/unk2_b_split090.c \
	src/overlays/field/unk2_b_split092.c \
	src/overlays/field/unk2_b_split094.c \
	src/overlays/field/unk2_b_split096.c \
	src/overlays/field/unk2_b_split097.c \
	src/overlays/field/unk2_b_split098.c \
	src/overlays/field/field235.c \
	src/overlays/field/unk2_b_split099.c \
	src/overlays/field/unk2_b_split100.c \
	src/overlays/field/unk2_b_split101.c \
	src/overlays/field/unk2_b_split102.c \
	src/overlays/field/unk2_b_split103.c \
	src/overlays/field/field241.c \
	src/overlays/field/unk2_i_b_split020.c \
	src/overlays/field/field242.c \
	src/overlays/field/unk2_i_b_split021.c \
	src/overlays/field/field244.c \
	src/overlays/field/unk2_b_split104.c \
	src/overlays/field/unk2_b_split105.c \
	src/overlays/field/unk2_b_split106.c \
	src/overlays/field/unk2_b_split107.c \
	src/overlays/field/unk2_b_split108.c \
	src/overlays/field/unk2_b_split109.c \
	src/overlays/field/unk2_b_split110.c
overlay_field_gcc_272_cdk_g0_nosched_srcs := src/overlays/field/field2.c
overlay_field_gcc_272_cdk_g0_noexpand_srcs := src/overlays/field/field3.c
overlay_field_gcc_280_g0_srcs := \
	src/overlays/field/field87.c \
	src/overlays/field/field89.c \
	src/overlays/field/field91.c \
	src/overlays/field/field92.c \
	src/overlays/field/field95.c \
	src/overlays/field/field99.c \
	src/overlays/field/field100.c \
	src/overlays/field/field101.c \
	src/overlays/field/field102.c \
	src/overlays/field/field103.c \
	src/overlays/field/field104.c \
	src/overlays/field/field105.c \
	src/overlays/field/field106.c \
	src/overlays/field/field107.c \
	src/overlays/field/field109.c \
	src/overlays/field/field110.c \
	src/overlays/field/field116.c \
	src/overlays/field/field117.c \
	src/overlays/field/field119.c \
	src/overlays/field/field124.c \
	src/overlays/field/field125.c \
	src/overlays/field/field126.c \
	src/overlays/field/field127.c \
	src/overlays/field/field128.c \
	src/overlays/field/field129.c \
	src/overlays/field/field130.c \
	src/overlays/field/field131.c \
	src/overlays/field/field143.c \
	src/overlays/field/field145.c \
	src/overlays/field/field146.c \
	src/overlays/field/field148.c \
	src/overlays/field/field149.c \
	src/overlays/field/field150.c \
	src/overlays/field/field151.c \
	src/overlays/field/field152.c \
	src/overlays/field/field154.c \
	src/overlays/field/field155.c \
	src/overlays/field/field156.c \
	src/overlays/field/field157.c \
	src/overlays/field/field159.c \
	src/overlays/field/field161.c \
	src/overlays/field/field163.c \
	src/overlays/field/field166.c \
	src/overlays/field/field167.c \
	src/overlays/field/field169.c \
	src/overlays/field/field170.c \
	src/overlays/field/field171.c \
	src/overlays/field/field172.c \
	src/overlays/field/field173.c \
	src/overlays/field/field174.c \
	src/overlays/field/field175.c \
	src/overlays/field/field176.c \
	src/overlays/field/field177.c \
	src/overlays/field/field179.c \
	src/overlays/field/field180.c \
	src/overlays/field/field181.c \
	src/overlays/field/field182.c \
	src/overlays/field/field183.c \
	src/overlays/field/field184.c \
	src/overlays/field/field186.c \
	src/overlays/field/field188.c \
	src/overlays/field/field190.c \
	src/overlays/field/field191.c \
	src/overlays/field/field193.c \
	src/overlays/field/field197.c \
	src/overlays/field/field199.c \
	src/overlays/field/field201.c \
	src/overlays/field/field208.c \
	src/overlays/field/field210.c \
	src/overlays/field/field211.c \
	src/overlays/field/field212.c \
	src/overlays/field/field213.c \
	src/overlays/field/field214.c \
	src/overlays/field/field215.c \
	src/overlays/field/field217.c \
	src/overlays/field/field218.c \
	src/overlays/field/field219.c \
	src/overlays/field/field220.c \
	src/overlays/field/field221.c \
	src/overlays/field/field222.c \
	src/overlays/field/field223.c \
	src/overlays/field/field224.c \
	src/overlays/field/field225.c \
	src/overlays/field/field226.c \
	src/overlays/field/field227.c \
	src/overlays/field/field228.c \
	src/overlays/field/field236.c \
	src/overlays/field/field237.c \
	src/overlays/field/field238.c \
	src/overlays/field/field239.c \
	src/overlays/field/field240.c \
	src/overlays/field/field243.c \
	src/overlays/field/field245.c \
	src/overlays/field/field246.c \
	src/overlays/field/field247.c \
	src/overlays/field/field248.c \
	src/overlays/field/field249.c
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
