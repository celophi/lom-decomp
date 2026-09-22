#include "wmap_land_transition.h"
#include "wmap_land_selection.h"

/** @brief Clear the candidate list and search for an available map entry. */
void func_8005909C(void)
{
extern s32 D_80139270;
extern s32 D_80139838[];
extern s32 func_8005CC50(s32);
extern s32 func_8005D8FC(void);
extern void func_8005CA3C(s32, s32 *);

    s32 i;

    D_80139270 = func_8005CC50(g_wmap_carousel_frame / 4);
    i = 0;
    do
    {
        D_80139838[i] = -1;
        i++;
    } while ((u32)i < 12U);
    i = 12;
    do
    {
        if (func_8005D8FC() != -1)
        {
            break;
        }
        g_wmap_carousel_frame += 4;
        func_8005CA3C(1, D_80139838);
        if (g_wmap_carousel_frame < 0)
        {
            g_wmap_carousel_frame = 47;
        }
        else if (g_wmap_carousel_frame >= 48)
        {
            g_wmap_carousel_frame = 0;
        }
        i--;
        g_wmap_carousel_rotation.vy = g_wmap_carousel_angles[g_wmap_carousel_frame].angle;
    } while (i != -1);
}
