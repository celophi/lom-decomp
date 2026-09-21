#include "common.h"
#include "saved_game.h"

extern u8 D_800432BD;
extern u8 *D_800D8B3C;

/**
 * @brief Compute the current world-map progress difference using the selected record flags.
 * @return Difference from the saved progress byte, or from the current progress byte.
 */
s32 func_8005D5D8(void)
{
    u32 flags = g_saved_game.words[0x434 / 4];
    u8 *data;
    s32 column;
    s32 row;
    s32 cell;
    u8 *entry;

    if (flags & 2)
    {
        data = D_800D8B3C;
        column = (flags >> 8) & 15;
        row = (flags >> 12) & 15;
        cell = column + row * 6;
        entry = data + cell * 12;
        if (*(u16 *)(entry + 0x10) & 4)
        {
            return *(s32 *)(data + 4) - g_saved_game.bytes[0x2E5] + 1;
        }
    }
    return *(s32 *)(D_800D8B3C + 4) - D_800432BD;
}
