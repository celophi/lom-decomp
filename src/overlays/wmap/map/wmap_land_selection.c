#include "../internal/wmap_land_layout.h"
#include "../internal/wmap_land_transition.h"
#include "../internal/wmap_land_selection.h"

extern s32 D_80139270;
extern s32 D_80139838[];

/** @brief Rebuild the artifact carousel and advance to an available selection. */
void wmap_reset_artifact_carousel(void)
{
    s32 selection_index;

    D_80139270 = wmap_build_artifact_list(g_wmap_carousel_frame / 4);
    for (selection_index = 0; (u32)selection_index < 12U; selection_index++)
    {
        D_80139838[selection_index] = -1;
    }
    selection_index = 12;
    while (selection_index != -1)
    {
        if (wmap_get_selected_artifact() != -1)
        {
            break;
        }
        g_wmap_carousel_frame += 4;
        wmap_scroll_artifact_list(1, D_80139838);
        if (g_wmap_carousel_frame < 0)
        {
            g_wmap_carousel_frame = 47;
        }
        else if (g_wmap_carousel_frame >= 48)
        {
            g_wmap_carousel_frame = 0;
        }
        selection_index--;
        g_wmap_carousel_rotation.vy = g_wmap_carousel_angles[g_wmap_carousel_frame].angle;
    }
}
