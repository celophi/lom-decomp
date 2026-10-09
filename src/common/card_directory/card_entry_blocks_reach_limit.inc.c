#include "common/card_directory.h"
#include "common/card_menu.h"

/**
 * @brief Check whether fewer than two card blocks remain free.
 * @return 1 when the used blocks reach CARD_USED_BLOCK_LIMIT, 0 otherwise.
 */
inline s32 card_entry_blocks_reach_limit(void)
{
    s32 entry_index;
    s32 used_blocks;

    used_blocks = 0;
    for (entry_index = 0; entry_index < g_card_entry_state; entry_index++)
    {
        used_blocks += g_card_entries[g_card_slot][entry_index].size / CARD_BLOCK_BYTES;
    }
    return used_blocks >= CARD_USED_BLOCK_LIMIT;
}
