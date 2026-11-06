#include "../internal/wmap_land_effect_00.h"
#include "../internal/wmap_land_effect_01.h"
#include "../internal/wmap_land_effect_02.h"
#include "../internal/wmap_land_effect_03.h"
#include "../internal/wmap_land_effect_04.h"
#include "../internal/wmap_land_effect_05.h"
#include "../internal/wmap_land_effect_07.h"
#include "../internal/wmap_land_effect_08.h"
#include "../internal/wmap_land_effect_09.h"
#include "../internal/wmap_land_effect_10.h"
#include "../internal/wmap_land_effect_11.h"
#include "../internal/wmap_land_effect_12.h"
#include "../internal/wmap_land_effect_13.h"
#include "../internal/wmap_land_effect_15.h"
#include "../internal/wmap_land_effect_16.h"
#include "../internal/wmap_land_effect_17.h"
#include "../internal/wmap_land_effect_18.h"
#include "../internal/wmap_land_effect_19.h"
#include "../internal/wmap_land_effect_21.h"
#include "../internal/wmap_land_effect_22.h"
#include "../internal/wmap_land_effect_23.h"
#include "../internal/wmap_land_effect_24.h"
#include "../internal/wmap_land_effect_25.h"
#include "../internal/wmap_land_effect_26.h"
#include "../internal/wmap_land_effect_27.h"
#include "../internal/wmap_land_effect_30.h"
#include "../internal/wmap_land_effect_31.h"
#include "../internal/wmap_land_effect_32.h"
#include "../internal/wmap_land_effect_33.h"
#include "../internal/wmap_land_effect_loader.h"
#include "../internal/wmap_map_display.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_sequence_runtime.h"
#include "main/cdrom.h"

/**
 * @brief Upload the selected effect resources and register its callback.
 * @param selection Land effect to start; selects its land display and run sequence.
 */
void wmap_start_land_effect(u32 selection)
{
    g_wmap_land_image_cache_missed = 0;
    cdrom_wait_queue_empty();
    wmap_upload_tim(g_wmap_effect_texture_buffer_0);
    wmap_upload_tim(g_wmap_effect_texture_buffer_1);

    if (selection != 2)
    {
        wmap_upload_tim(g_wmap_effect_texture_buffer_2);
    }

    if (selection == 0x16)
    {
        wmap_resolve_land_image(&g_wmap_land_display[16]);
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
