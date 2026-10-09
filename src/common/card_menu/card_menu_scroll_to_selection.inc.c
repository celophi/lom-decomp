#include "common/card_menu.h"

/**
 * @brief Retarget the list scroll so the selected row stays in the window.
 */
inline void card_menu_scroll_to_selection(void)
{
    s32 row_y;
    s32 relative_y;

    row_y = g_card_menu_selected_row * CARD_MENU_ENTRY_ROW_HEIGHT;
    relative_y = row_y - g_card_menu_scroll_y;
    if (relative_y > CARD_MENU_LIST_HEIGHT - CARD_MENU_ENTRY_ROW_HEIGHT)
    {
        g_card_menu_scroll_target_y = row_y - CARD_MENU_LIST_LAST_ROW_Y;
        g_card_menu_scroll_frames = CARD_MENU_SCROLL_FRAMES;
    }
    if (relative_y < 0)
    {
        g_card_menu_scroll_target_y = g_card_menu_selected_row * CARD_MENU_ENTRY_ROW_HEIGHT;
        g_card_menu_scroll_frames = CARD_MENU_SCROLL_FRAMES;
    }
}
