#include "common.h"
#include "saved_game.h"

extern u8 D_800435E0;
extern u16 *D_800D8B3C;
extern u16 g_music_track_index;

/**
 * @brief Read the active map coordinates, falling back to the default pair.
 * @param row Receives the low-nibble coordinate or default first coordinate.
 * @param column Receives the high-nibble coordinate or default second coordinate.
 * @return Low seven bits of the current map status.
 */
s32 func_8005D850(s32 *row, u32 *column)
{
    if (!(g_saved_game.words[0x2F0 / 4] & 2))
    {
        *row = D_800D8B3C[0];
        *column = D_800D8B3C[1];
    }
    else
    {
        *row = ((SavedGame *)(g_music_track_index * 12 + g_saved_game.bytes))->bytes[0x2F1] & 15;
        *column = ((SavedGame *)(g_music_track_index * 12 + g_saved_game.bytes))->bytes[0x2F1] >> 4;
    }
    return D_800435E0 & 0x7F;
}
