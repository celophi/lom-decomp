#include "common.h"
#include "saved_game.h"

/**
 * @brief Test whether saved flags 0, 9 through 14, 24, 27, or 28 are set.
 * @return One if any of the selected flags is set, otherwise zero.
 */
s32 func_8005D9B4(void)
{
    s32 index;
    s32 found;
    s32 word_index;
    u32 bits;
    u32 flags;
    SavedGame *word_base;

    found = 0;
    index = 0;
    do
    {
        word_index = index / 32;
        word_base = (SavedGame *)(word_index * 4 + g_saved_game.bytes);
        bits = 1U << (index - word_index * 32);
        flags = word_base->words[0x2E8 / 4];
        if ((flags & bits) &&
            ((u32)(index - 9) < 6 || index == 24 || index == 0 || index == 27 || index == 28))
        {
            found = 1;
        }
        index++;
    } while (index < 64);
    return found;
}
