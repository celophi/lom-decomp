#include "common/card_menu.h"

/**
 * @brief Reset the browser and read the first entry on a card.
 * @param page Memory-card slot to scan.
 * @return 1 if an entry was read, 0 if the directory search failed.
 */
s32 card_menu_begin_entry_scan(s32 page)
{
    CardSearchPattern pattern;

    strcpy(pattern.text, CARD_SEARCH_PATTERN);
    g_card_menu_selected_row = 0;
    g_card_menu_scroll_frames = 0;
    g_card_menu_scroll_target_y = 0;
    g_card_menu_scroll_y = 0;
    g_card_entry_state = 0;
    pattern.device.characters.slot += page;
    if (firstfile(pattern.text, &g_card_entries[page][0]) != 0)
    {
        field_flag_known_save(g_card_entries[page][g_card_entry_state].name);
        g_card_entry_state += 1;
        return 1;
    }
    return 0;
}
