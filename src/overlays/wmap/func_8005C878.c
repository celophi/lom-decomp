/* Partial WMAP decompilation: 97.222220% (gcc280_g0). */
#include "common.h"
#include "saved_game.h"

extern s32 D_800D9130;
extern s32 g_layout_sub_mode;

/**
 * @brief Find the next pending world-map flag and consume nonpersistent entries.
 * @return The flag index, or -1 when no pending entry remains.
 */
s32 func_8005C878(void)
{
    s32 bank;
    s32 mask;
    SavedGame *flags;

    D_800D9130++;
    while (D_800D9130 < 64)
    {
        bank = D_800D9130 / 32;
        flags = (SavedGame *)(g_saved_game.words + bank);
        mask = 1 << (D_800D9130 - (bank << 5));
        if (flags->words[0x2E8 / 4] & mask)
        {
            if (D_800D9130 == 0)
            {
                g_layout_sub_mode = 31;
            }
            if ((u32)(D_800D9130 - 2) >= 2U &&
                D_800D9130 != 22 && D_800D9130 != 23 &&
                D_800D9130 != 9 && D_800D9130 != 10 &&
                D_800D9130 != 11 && D_800D9130 != 12 &&
                D_800D9130 != 13 && D_800D9130 != 14 &&
                D_800D9130 != 24 && D_800D9130 != 0 &&
                D_800D9130 != 27 && D_800D9130 != 28)
            {
                flags->words[0x2E8 / 4] &= ~(1 << (D_800D9130 % 32));
            }
            return D_800D9130;
        }
        D_800D9130++;
    }
    return -1;
}
