#include "common.h"

#include "saved_game.h"

/**
 * @brief Test whether a saved world-map record has bit zero set and bit one clear.
 * @param record_index Index of the 12-byte record.
 * @return One when both flag conditions hold, otherwise zero.
 */
s32 func_8005D810(s32 record_index)
{
    u32 flags = g_saved_game.bytes[record_index * 12 + 0x2F0];
    if (flags & 1)
    {
        return ((flags >> 1) & 1) ^ 1;
    }
    return 0;
}
