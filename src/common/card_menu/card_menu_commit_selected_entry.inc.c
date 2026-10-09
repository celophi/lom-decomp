#include "common/card_menu.h"

/**
 * @brief Build the selected save path and start reading its details.
 */
void card_menu_commit_selected_entry(void)
{
    CardEntryPath card_path;

    if (g_card_entry_state == 0)
    {
        g_card_menu_selection_status = CARD_MENU_SELECTION_EMPTY_CARD;
        return;
    }
    if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][g_card_menu_selected_row].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) == 0)
    {
        g_card_menu_selection_status = CARD_MENU_SELECTION_NEW_SAVE;
        return;
    }
    strcpy(card_path.text, CARD_DEVICE_PREFIX);
    strcat(card_path.text, g_card_entries[g_card_slot][g_card_menu_selected_row].name);
    card_path.device.characters.slot += g_card_slot;
    g_card_menu_selection_status = CARD_MENU_SELECTION_NONE;
    strcpy(g_card_selected_save_path, card_path.text);
    g_card_step = g_card_steps_read_selected_header;
    if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][g_card_menu_selected_row].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
    {
        g_card_menu_selected_entry_extended = 1;
    }
    else
    {
        g_card_menu_selected_entry_extended = 0;
    }
    g_card_menu_io_busy = 1;
}
