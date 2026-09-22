#include "common.h"

#include "saved_game.h"

/**
 * @brief Read bit two of a saved world-map record's flags.
 * @param record_index Index of the 12-byte record.
 * @return One when bit two is set, otherwise zero.
 */
s32 func_8005D528(s32 record_index)
{
    return (g_saved_game.bytes[record_index * 12 + 0x2F0] >> 2) & 1;
}
