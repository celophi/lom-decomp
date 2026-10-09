#include "common/card_directory.h"
#include "common/card_menu.h"

/**
 * @brief Clear the save ranks and reset the rank count.
 */
inline void card_reset_entry_ranks(void)
{
    s32 entry_index;
    s32 empty_rank;

    g_card_rank_count = 40;
    empty_rank = -1;
    for (entry_index = CARD_SAVE_SLOTS - 1; entry_index >= 0; entry_index--)
    {
        g_card_entry_ranks[entry_index] = empty_rank;
    }
}
