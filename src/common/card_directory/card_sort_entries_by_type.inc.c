#include "common/card_directory.h"
#include "common/card_menu.h"

/**
 * @brief Reorder the current card's directory by save type and suffix.
 * @note Legend of Mana saves come first, then PocketStation saves (each grouped by
 *       suffix), then the new-save placeholder, then every other file; entries keep
 *       their directory order within a group.
 */
void card_sort_entries_by_type(void)
{
    struct DIRENTRY sorted[CARD_DIRECTORY_ENTRY_COUNT];
    s32 output_index = 0;
    s32 suffix;
    s32 entry_index;

    for (suffix = 0; suffix < CARD_MENU_ENTRY_GROUP_COUNT; suffix++)
    {
        for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
        {
            if (g_card_entry_suffix_values[entry_index] == suffix &&
                strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                bcopy((u8*)&g_card_entries[g_card_slot][entry_index], (u8*)&sorted[output_index], sizeof(struct DIRENTRY));
                output_index++;
            }
        }
    }

    for (suffix = 0; suffix < CARD_MENU_ENTRY_GROUP_COUNT; suffix++)
    {
        for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
        {
            if (g_card_entry_suffix_values[entry_index] == suffix &&
                strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) == 0)
            {
                bcopy((u8*)&g_card_entries[g_card_slot][entry_index], (u8*)&sorted[output_index], sizeof(struct DIRENTRY));
                output_index++;
            }
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) == 0)
        {
            bcopy((u8*)&g_card_entries[g_card_slot][entry_index], (u8*)&sorted[output_index], sizeof(struct DIRENTRY));
            output_index++;
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_lom_save_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) != 0 &&
            strncmp(g_lom_pocketstation_filename_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_SAVE_FILENAME_PREFIX_LENGTH) != 0 &&
            strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) != 0)
        {
            bcopy((u8*)&g_card_entries[g_card_slot][entry_index], (u8*)&sorted[output_index], sizeof(struct DIRENTRY));
            output_index++;
        }
    }

    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        bcopy((u8*)&sorted[entry_index], (u8*)&g_card_entries[g_card_slot][entry_index], sizeof(struct DIRENTRY));
    }
}
