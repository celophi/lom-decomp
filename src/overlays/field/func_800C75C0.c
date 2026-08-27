#include "common.h"

extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Clears the active flag for the gosub-selected small history slot.
 *
 * Uses the first gosub result to select a 0x60-byte record and clears bit 30
 * of its word at offset 0x2F38. Out-of-range indices trigger the corresponding
 * song-parameter command instead.
 *
 * @note The implicit akao_set_song_params declaration is required for the
 *       matching call convention used by this field code.
 */
void func_800C75C0(void)
{
    s32 index;
    u8 *base;
    u8 *record;

    index = g_gosub_result_values[0];
    if (index < 5)
    {
        base = g_menuLayoutBuffer;
        record = base + index * 0x60;
        *(u32 *)(record + 0x2F38) &= 0xBFFFFFFF;
    }
    else
    {
        akao_set_song_params(0x8002, 0x32, index, 0);
    }
}
