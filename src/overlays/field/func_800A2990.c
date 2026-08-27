#include "common.h"

extern u8 D_80117E98[][0x10];
extern s32 D_80117E88[];

/**
 * @brief Compacts a row's byte table, dropping the first @p start entries.
 *
 * When @p start is below 0x10, shifts @c D_80117E98[row] entries [start, 0x10)
 * down to the front of the row, then decrements the row's count in
 * @c D_80117E88 by @p start.
 *
 * @param row Row index into @c D_80117E98 / @c D_80117E88.
 * @param start Number of leading entries to drop (also subtracted from the
 *              count).
 */
void func_800A2990(s32 row, s32 start)
{
    u8 (*table)[0x10];
    u8 *base;
    s32 i;
    s32 j;

    j = 0;
    if (start < 0x10)
    {
        table = D_80117E98;
        base = table[row];
        i = start;
        do
        {
            base[j++] = base[i++];
        } while (i < 0x10);
    }

    D_80117E88[row] -= start;
}
