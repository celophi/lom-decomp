#include "wmap_map_display.h"
#include "wmap_land_effect_loader.h"
#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "cdrom.h"

extern WmapLandDisplay D_80182508;
extern s32 D_801ADAF8;
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern u8 D_80193640[];
extern s32 wmap_land_effect_18_run(s32);
extern s32 wmap_land_effect_04_run(s32);
extern s32 wmap_land_effect_01_run(s32);
extern s32 wmap_land_effect_13_run(s32);
extern s32 wmap_land_effect_26_run(s32);
extern s32 wmap_land_effect_32_run(s32);
extern s32 wmap_land_effect_15_run(s32);
extern s32 wmap_land_effect_02_run(s32);
extern s32 wmap_land_effect_07_run(s32);
extern s32 wmap_land_effect_30_run(s32);
extern s32 wmap_land_effect_12_run(s32);
extern s32 wmap_land_effect_11_run(s32);
extern s32 wmap_land_effect_03_run(s32);
extern s32 wmap_land_effect_21_run(s32);
extern s32 wmap_land_effect_17_run(s32);
extern s32 wmap_land_effect_05_run(s32);
extern s32 wmap_land_effect_10_run(s32);
extern s32 wmap_land_effect_00_run(s32);
extern s32 wmap_land_effect_08_run(s32);
extern s32 wmap_land_effect_09_run(s32);
extern s32 wmap_land_effect_16_run(s32);
extern s32 wmap_land_effect_27_run(s32);
extern s32 wmap_land_effect_23_run(s32);
extern s32 wmap_land_effect_22_run(s32);
extern s32 wmap_land_effect_19_run(s32);
extern s32 wmap_land_effect_25_run(s32);
extern s32 wmap_land_effect_31_run(s32);
extern s32 wmap_land_effect_24_run(s32);
extern s32 wmap_land_effect_33_run(s32);

/** @brief Upload the selected effect resources and register its callback. */
void func_800591A8(u32 selection)
{
    D_801ADAF8 = 0;
    cdrom_wait_queue_empty();
    func_800651B4(D_80182E40);
    func_800651B4(D_8018B240);
    if (selection != 2)
    {
        func_800651B4(D_80193640);
    }
    if (selection == 0x16)
    {
        wmap_resolve_land_image(&D_80182508);
    }
    else
    {
        wmap_resolve_land_image(&g_wmap_land_display[selection]);
    }
    switch (selection)
    {
    case 33:
        wmap_start_sequence(wmap_land_effect_33_run);
        break;
    case 24:
        wmap_start_sequence(wmap_land_effect_24_run);
        break;
    case 31:
        wmap_start_sequence(wmap_land_effect_31_run);
        break;
    case 25:
        wmap_start_sequence(wmap_land_effect_25_run);
        break;
    case 22:
        wmap_start_sequence(wmap_land_effect_22_run);
        break;
    case 19:
        wmap_start_sequence(wmap_land_effect_19_run);
        break;
    case 23:
        wmap_start_sequence(wmap_land_effect_23_run);
        break;
    case 27:
        wmap_start_sequence(wmap_land_effect_27_run);
        break;
    case 16:
        wmap_start_sequence(wmap_land_effect_16_run);
        break;
    case 0:
        wmap_start_sequence(wmap_land_effect_00_run);
        break;
    case 9:
        wmap_start_sequence(wmap_land_effect_09_run);
        break;
    case 8:
        wmap_start_sequence(wmap_land_effect_08_run);
        break;
    case 10:
        wmap_start_sequence(wmap_land_effect_10_run);
        break;
    case 5:
        wmap_start_sequence(wmap_land_effect_05_run);
        break;
    case 17:
        wmap_start_sequence(wmap_land_effect_17_run);
        break;
    case 21:
        wmap_start_sequence(wmap_land_effect_21_run);
        break;
    case 3:
        wmap_start_sequence(wmap_land_effect_03_run);
        break;
    case 11:
        wmap_start_sequence(wmap_land_effect_11_run);
        break;
    case 12:
        wmap_start_sequence(wmap_land_effect_12_run);
        break;
    case 30:
        wmap_start_sequence(wmap_land_effect_30_run);
        break;
    case 7:
        wmap_start_sequence(wmap_land_effect_07_run);
        break;
    case 2:
        wmap_start_sequence(wmap_land_effect_02_run);
        break;
    case 15:
        wmap_start_sequence(wmap_land_effect_15_run);
        break;
    case 32:
        wmap_start_sequence(wmap_land_effect_32_run);
        break;
    case 26:
        wmap_start_sequence(wmap_land_effect_26_run);
        break;
    case 1:
        wmap_start_sequence(wmap_land_effect_01_run);
        break;
    case 18:
        wmap_start_sequence(wmap_land_effect_18_run);
        break;
    case 4:
        wmap_start_sequence(wmap_land_effect_04_run);
        break;
    case 13:
        wmap_start_sequence(wmap_land_effect_13_run);
        break;
    default:
        wmap_start_sequence(wmap_run_land_entry);
        break;
    }
}
