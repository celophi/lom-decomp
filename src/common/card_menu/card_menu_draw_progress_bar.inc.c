#include "common/card_menu.h"

/**
 * @brief Draw the transfer progress bar: a gradient across the top of the message
 *        window that fills over CARD_MENU_PROGRESS_FULL_TICKS VSyncs.
 * @param quad Primitive-buffer cursor the bar is written to.
 * @param ot Ordering-table entry the bar is linked into.
 * @return Primitive-buffer cursor after the bar, or @p quad while no transfer is running.
 */
inline void* card_menu_draw_progress_bar(POLY_G4* quad, u_long* ot)
{
    s32 elapsed;
    s32 width;

    if (g_card_menu_progress_bar_active != 0)
    {
        elapsed = VSync(-1) - g_card_menu_progress_start_tick;
        if (elapsed > CARD_MENU_PROGRESS_FULL_TICKS)
        {
            elapsed = CARD_MENU_PROGRESS_FULL_TICKS;
        }
        width = elapsed * CARD_MENU_MESSAGE_WIDTH;
        SET_BGR0_PACKED(quad, CARD_MENU_PROGRESS_TOP_LEFT_COLOR);
        SET_POLY_G4_BGR1_PACKED(quad, CARD_MENU_PROGRESS_TOP_RIGHT_COLOR);
        SET_POLY_G4_BGR3_PACKED(quad, CARD_MENU_PROGRESS_BOTTOM_RIGHT_COLOR);
        SET_POLY_G4_BGR2_PACKED(quad, CARD_MENU_PROGRESS_BOTTOM_LEFT_COLOR);
        setPolyG4(quad);
        quad->x0 = quad->x2 = 0;
        quad->x1 = quad->x3 = width / CARD_MENU_PROGRESS_FULL_TICKS;
        quad->y0 = quad->y1 = 0;
        quad->y2 = quad->y3 = CARD_MENU_MESSAGE_HEIGHT;
        addPrim(ot, quad);
        quad++;
    }
    return quad;
}
