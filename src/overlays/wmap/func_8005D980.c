#include "common.h"

extern s32 D_800D00CC[];

/**
 * @brief Look up a value in the seven-column world-map table.
 * @param row Table row.
 * @param column Table column.
 * @return Table value, or zero if either index is negative.
 */
s32 func_8005D980(s32 row, s32 column)
{
    s32 value = 0;
    if (row >= 0 && column >= 0)
    {
        value = D_800D00CC[row * 7 + column];
    }
    return value;
}
