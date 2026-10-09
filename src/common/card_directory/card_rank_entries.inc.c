#include "common/card_directory.h"
#include "common/card_menu.h"

/**
 * @brief Rank saves by serial number and assign the next suffix to the new-save entry.
 * @return Index of the save with the highest serial, or zero if none is found.
 */
s32 card_rank_entries(void)
{
    s32 entry_index;
    s32 previous_index;
    s32 higher_count;
    s32 next_rank;
    s32 maximum;
    s32 max_suffix;

    parse_entry_fields();
    maximum = -1;
    card_sort_entries_by_type();
    max_suffix = parse_entry_fields();
    card_reset_entry_ranks();
    next_rank = 1;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (g_card_entry_fields[g_card_slot][entry_index] >= 0)
        {
            if (g_card_entry_fields[g_card_slot][entry_index] >= maximum)
            {
                g_card_entry_ranks[entry_index] = next_rank;
                maximum = g_card_entry_fields[g_card_slot][entry_index];
                next_rank++;
            }
            else
            {
                higher_count = 0;
                for (previous_index = 0; previous_index < entry_index; previous_index++)
                {
                    if (g_card_entry_fields[g_card_slot][entry_index] < g_card_entry_fields[g_card_slot][previous_index])
                    {
                        higher_count++;
                        g_card_entry_ranks[previous_index]++;
                    }
                }
                g_card_entry_ranks[entry_index] = next_rank - higher_count;
                next_rank++;
            }
        }
    }
    g_card_rank_count = next_rank;
    /* Reuse next_rank as the running maximum and maximum as its index. */
    next_rank = -1;
    maximum = 0;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (next_rank < g_card_entry_fields[g_card_slot][entry_index])
        {
            next_rank = g_card_entry_fields[g_card_slot][entry_index];
            maximum = entry_index;
        }
    }
    g_card_entry_value_limit = next_rank + 1;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        if (strncmp(g_new_save_entry_prefix, g_card_entries[g_card_slot][entry_index].name, CARD_MENU_NEW_SAVE_NAME_LENGTH) == 0)
        {
            g_card_entry_suffix_values[entry_index] = max_suffix + 1;
            break;
        }
    }
    return maximum;
}
