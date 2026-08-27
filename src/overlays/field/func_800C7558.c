#include "common.h"

extern u8 g_menuLayoutBuffer[];

/**
 * @brief Clears the active flag for the selected small history slot.
 *
 * Uses the small-history index at offset 0x2EF0 to select a 0x60-byte record
 * and clears bit 30 of its word at offset 0x2F38. Out-of-range indices trigger
 * the corresponding song-parameter command instead.
 *
 * @note The implicit akao_set_song_params declaration is required for the
 *       matching call convention used by this field code.
 */
void func_800C7558(void)
{
    s32 index;
    u8 *base;
    u8 *record;

    base = g_menuLayoutBuffer;
    index = *(s32 *)(base + 0x2EF0);
    if (index < 5)
    {
        record = base + index * 0x60;
        *(u32 *)(record + 0x2F38) &= 0xBFFFFFFF;
    }
    else
    {
        akao_set_song_params(0x8002, 0x32, index, 0);
    }
}
