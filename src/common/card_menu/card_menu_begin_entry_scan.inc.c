#include "common/card_menu.h"

/**
 * @brief Reset the browser and read the first entry on a card.
 * @param slot Memory-card slot to scan.
 * @return 1 if an entry was read, 0 if the directory search failed.
 */
s32 card_menu_begin_entry_scan(s32 slot)
{
    CardSearchPattern pattern;

    strcpy(pattern.text, CARD_SEARCH_PATTERN);
    g_card_menu_selected_row = 0;
    g_card_menu_scroll_frames = 0;
    g_card_menu_scroll_target_y = 0;
    g_card_menu_scroll_y = 0;
    g_card_entry_state = 0;
    pattern.device.characters.slot += slot;
    if (firstfile(pattern.text, &g_card_entries[slot][0]) != 0)
    {
        field_flag_known_save(g_card_entries[slot][g_card_entry_state].name);
        g_card_entry_state++;
        return 1;
    }
    return 0;
}
